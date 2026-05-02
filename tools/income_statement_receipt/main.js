const csv     = require('csvtojson');
const receipt = require('receipt');
const sun_country = require('sun-country');

receipt.config.currency = "£";
receipt.config.width    = 50;
receipt.config.ruler    = "=";

var vat_threshold         = 85000;
var total_payment_count   = 0;
var total_payment_amount  = 0;
var total_vat_liable      = 0;
var total_payment_refunds = 0;
var total_payment_fees    = 0;
var payments              = [];
var payments_by_country   = [];
var income_by_month       = [];
var receipt_data          = []
var refunds_by_month      = [];
var fees_by_month         = [];
var total_disputes_amount = 0;
var disputes_by_month     = [];

async function dump_csv_to_json(){
  const json_tbl = await csv().fromFile("unified_payments.csv");

  // dump entries to payments json table
  for(var e in json_tbl)
    payments.push(json_tbl[e]);

  // Correct table order as we want oldest to new
  payments.reverse();
}

async function dump_by_countries(){
  for(var e in payments){
    e = payments[e];

    var country = e["Card Address Country"];
    if(country == undefined || country == "")
      country = "OTHER";

    if(country == "GB")
      country = "UK";

    var amount          = Number(Number(e["Converted Amount"]).toFixed(2));
    var fees            = Number(Number(e["Fee"]).toFixed(2));
    var refunded        = Number(Number(e["Converted Amount Refunded"]).toFixed(2));
    var disputed_amount = Number(Number(e["Disputed Amount"]).toFixed(2));
    var disputed_reason = e["Dispute Status"];
    var month           = e["Created (UTC)"];
    var status          = e["Status"];
        month           = new Date(month).toLocaleString('default', { month: 'long' });

    if(status != "Paid" && status != "Refunded")
      continue;

    if(disputed_amount > 0 && disputed_reason == "lost"){
      //amount                -= disputed_amount;
      total_disputes_amount += disputed_amount;

      if(disputes_by_month[month] == undefined)
        disputes_by_month[month] = disputed_amount;
      else
        disputes_by_month[month] += disputed_amount;
    }

    // Subtract any refunds given to the customer
    if(refunded > 0){
      //amount -= refunded;
      total_payment_refunds += refunded;

      if(refunds_by_month[month] == undefined)
        refunds_by_month[month] = refunded;
      else
        refunds_by_month[month] += refunded;
    }

    // Subtract any fees given to the customer
    if(fees > 0){
      total_payment_fees += fees;

      if(fees_by_month[month] == undefined)
        fees_by_month[month] = fees;
      else
        fees_by_month[month] += fees;
    }

    // If the amount is zero or below then the payment was terminated
    if(amount <= 0)
      continue;

    if(income_by_month[month] == undefined)
      income_by_month[month] = amount;
    else
      income_by_month[month] += amount;

    if(payments_by_country[country] == undefined)
      payments_by_country[country] = {qty: 1, income: amount, income_fees: fees, payments: [e]};
    else{
      payments_by_country[country].qty++;
      payments_by_country[country].income       += amount;
      payments_by_country[country].income_fees  += fees;
      payments_by_country[country].payments.push(e);
    }

    total_payment_amount += amount;
    total_payment_count++;

    if(total_payment_amount >= vat_threshold && country === "UK")
      total_vat_liable += amount;

    payments_by_country[country].income      = Number(payments_by_country[country].income.toFixed(2));
    payments_by_country[country].income_fees = Number(payments_by_country[country].income_fees.toFixed(2));
  }

  // HACK: Resort table and fill back into table from scratch, its ghetto but it works
  const resorted_pbc = Object.entries(payments_by_country).map(([key, value]) => ({...value, key: key})).sort((a, b) => b.income - a.income);
  payments_by_country = [];
  for(var e in resorted_pbc){
    e = resorted_pbc[e];

    payments_by_country[e.key] = e;
  }
}

function format_currency(x){
  return new Intl.NumberFormat('en-GB', { style: 'currency', currency: 'GBP' }).format(Number(x.toFixed(2)));
}

async function run(){
  await dump_csv_to_json();
  await dump_by_countries();

  // Create receipt header
  receipt_data.push({
    type: 'text',
    value: [
      "REVENUE STATEMENT RECEIPT",
      "",
      `KIERAN WATKINS T/A ("RIJIN")`,
      "79 GENOA COURT, ANDOVER, HAMPSHIRE",
      "",
      "FOR THE TAX YEAR 22/23",
    ],
    align: "center"
  });
  receipt_data.push({type: "empty"});
  receipt_data.push({type: "ruler"});

  let date_time = new Date();
  receipt_data.push({
    type: 'text',
    value: [
      "THIS DOCUMENT HAS BEEN AUTOMATICALLY GENERATED USING COMPLETE AND FULL ACCOUNTING RECORDS FROM ALL CUSTOMERS DURING THE TAX YEAR STATED ABOVE.",
      "",
      `DOCUMENT GENERATED: ${date_time.toISOString().slice(0,10)}`,
      "",
      "THIS DOCUMENT DOES NOT INCLUDE EXPENSES INCURRED."
    ],
    align: "center",
    padding: 0
  });
  receipt_data.push({ type: 'ruler'});
  receipt_data.push({type: "empty"});

  // Income by month
  {
    receipt_data.push({type: "empty"});
    receipt_data.push({
      type: 'text',
      value: [
        "REVENUE BREAKDOWN BY MONTH",
        "OVER 12 MONTH PERIOD"
      ],
      align: "center"
    });
    receipt_data.push({ type: 'ruler'});

    var sales_receipt_table = [];
    var country = new Country;
    for(var e in income_by_month){
      var income  = format_currency(income_by_month[e]);
      var perc    = ((income_by_month[e] / total_payment_amount) * 100).toFixed(2);

      sales_receipt_table.push({ name: `${e}`, value: `${income} (${perc}%)`});
    }

    receipt_data.push({ type: 'properties', lines: sales_receipt_table});
    receipt_data.push({ type: 'ruler'});
    receipt_data.push({
      type: 'text',
      value: [
        `FEES:\t\t${format_currency(total_payment_fees)}`,
        `REFUNDS:\t\t\t\t${format_currency(total_payment_refunds)}`,
        `DISPUTES:  ${format_currency(total_disputes_amount)}`,
        `TOTAL: ${format_currency(total_payment_amount)}`,
        `TOTAL LESS FEES/REFUNDS/DISPUTES: ${format_currency(total_payment_amount - (total_payment_refunds + total_payment_fees + total_disputes_amount))}`,
      ],
      align: "right"
    });
    receipt_data.push({ type: 'ruler'});

    receipt_data.push({type: "empty"});
  }

  // Income by country
  {
    receipt_data.push({type: "empty"});
    receipt_data.push({
      type: 'text',
      value: [
        "REVENUE BREAKDOWN BY COUNTRY",
        "OVER 12 MONTH PERIOD"
      ],
      align: "center"
    });
    receipt_data.push({ type: 'ruler'});

    var sales_receipt_table = [];
    var country = new Country;
    for(var e in payments_by_country){
      var country_name = e === "OTHER" ? "Other/Asia" : country.get(e).name;
      var income       = payments_by_country[e].income;
      var perc         = ((payments_by_country[e].qty / total_payment_count) * 100).toFixed(2);

      sales_receipt_table.push({ name: `${country_name}`, value: `${format_currency(income)} (${perc}%)`});
    }

    receipt_data.push({ type: 'properties', lines: sales_receipt_table});
    receipt_data.push({ type: 'ruler'});
    receipt_data.push({
      type: 'text',
      value: [
        `FEES:\t\t${format_currency(total_payment_fees)}`,
        `REFUNDS:\t\t\t\t${format_currency(total_payment_refunds)}`,
        `DISPUTES:  ${format_currency(total_disputes_amount)}`,
        `TOTAL: ${format_currency(total_payment_amount)}`,
        `TOTAL LESS FEES/REFUNDS/DISPUTES: ${format_currency(total_payment_amount - (total_payment_refunds + total_payment_fees + total_disputes_amount))}`,
      ],
      align: "right"
    });
    receipt_data.push({ type: 'ruler'});
  }

  // Fees by month
  {
    receipt_data.push({type: "empty"});
    receipt_data.push({type: "empty"});
    receipt_data.push({
      type: 'text',
      value: [
        "FEES BREAKDOWN BY MONTH",
        "OVER 12 MONTH PERIOD"
      ],
      align: "center"
    });
    receipt_data.push({ type: 'ruler'});

    var sales_receipt_table = [];
    var country = new Country;
    for(var e in fees_by_month){
      sales_receipt_table.push({ name: `${e}`, value: `${format_currency(fees_by_month[e])}`});
    }

    receipt_data.push({ type: 'properties', lines: sales_receipt_table});
    receipt_data.push({ type: 'ruler'});
    receipt_data.push({
      type: 'text',
      value: [
        `TOTAL FEES: ${format_currency(total_payment_fees)}`,
      ],
      align: "right"
    });
    receipt_data.push({ type: 'ruler'});
  }

  // Refunds by month
  {
    receipt_data.push({type: "empty"});
    receipt_data.push({type: "empty"});
    receipt_data.push({
      type: 'text',
      value: [
        "REFUNDS BREAKDOWN BY MONTH",
        "OVER 12 MONTH PERIOD"
      ],
      align: "center"
    });
    receipt_data.push({ type: 'ruler'});

    var sales_receipt_table = [];
    var country = new Country;
    for(var e in refunds_by_month){
      sales_receipt_table.push({ name: `${e}`, value: `${format_currency(refunds_by_month[e])}`});
    }

    receipt_data.push({ type: 'properties', lines: sales_receipt_table});
    receipt_data.push({ type: 'ruler'});
    receipt_data.push({
      type: 'text',
      value: [
        `TOTAL REFUNDS: ${format_currency(total_payment_refunds)}`,
      ],
      align: "right"
    });
    receipt_data.push({ type: 'ruler'});
  }

  // Disputes by month
  {
    receipt_data.push({type: "empty"});
    receipt_data.push({type: "empty"});
    receipt_data.push({
      type: 'text',
      value: [
        "DISPUTE BREAKDOWN BY MONTH",
        "OVER 12 MONTH PERIOD"
      ],
      align: "center"
    });
    receipt_data.push({ type: 'ruler'});

    var sales_receipt_table = [];
    var country = new Country;
    for(var e in disputes_by_month){
      sales_receipt_table.push({ name: `${e}`, value: `${format_currency(disputes_by_month[e])}`});
    }

    receipt_data.push({ type: 'properties', lines: sales_receipt_table});
    receipt_data.push({ type: 'ruler'});
    receipt_data.push({
      type: 'text',
      value: [
        `TOTAL DISPUTES: ${format_currency(total_disputes_amount)}`,
      ],
      align: "right"
    });
    receipt_data.push({ type: 'ruler'});
  }

  // VAT
  {
    receipt_data.push({type: "empty"});
    receipt_data.push({type: "empty"});
    receipt_data.push({
      type: 'text',
      value: [
        "VAT CALCULATION (ESTIMATED)",
        "UNITED KINGDOM CUSTOMERS ONLY",
        "&",
        "FOR PERSONAL RECORDS ONLY"
      ],
      align: "center"
    });
    receipt_data.push({ type: 'ruler'});
    {
      receipt_data.push({
        type: 'text',
        value: [
          `TOTAL LIABLE OVER ${format_currency(vat_threshold)}: ${format_currency(total_vat_liable)}`,
          `VAT @ 20%: ${format_currency(total_vat_liable * 0.2)}`,
        ],
        align: "right"
      });
    }
    receipt_data.push({ type: 'ruler'});
  }

  receipt_data.push({type: "empty"});
  receipt_data.push({type: "empty"});
  receipt_data.push({
    type: 'text',
    value: [
      "END OF GENERATED DOCUMENT"
    ],
    align: "center"
  });

  const output = receipt.create(receipt_data);
  console.log(output);
}
run();