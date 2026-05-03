namespace RijiNFeatureListGenerator {
  partial class main_form {
    /// <summary>
    /// Required designer variable.
    /// </summary>
    private System.ComponentModel.IContainer components = null;

    /// <summary>
    /// Clean up any resources being used.
    /// </summary>
    /// <param name="disposing">true if managed resources should be disposed; otherwise, false.</param>
    protected override void Dispose(bool disposing) {
      if (disposing && (components != null)) {
        components.Dispose();
      }
      base.Dispose(disposing);
    }

    #region Windows Form Designer generated code

    /// <summary>
    /// Required method for Designer support - do not modify
    /// the contents of this method with the code editor.
    /// </summary>
    private void InitializeComponent() {
      this.panel1 = new System.Windows.Forms.Panel();
      this.status_label = new System.Windows.Forms.Label();
      this.create_list_button = new System.Windows.Forms.Button();
      this.menu_file_path = new System.Windows.Forms.TextBox();
      this.import_menu_button = new System.Windows.Forms.Button();
      this.feature_list = new System.Windows.Forms.RichTextBox();
      this.file_dialog = new System.Windows.Forms.OpenFileDialog();
      this.supported_anticheat_textbox = new System.Windows.Forms.TextBox();
      this.label1 = new System.Windows.Forms.Label();
      this.prices_textbox = new System.Windows.Forms.TextBox();
      this.label2 = new System.Windows.Forms.Label();
      this.label3 = new System.Windows.Forms.Label();
      this.payment_methods = new System.Windows.Forms.TextBox();
      this.panel1.SuspendLayout();
      this.SuspendLayout();
      // 
      // panel1
      // 
      this.panel1.Controls.Add(this.label3);
      this.panel1.Controls.Add(this.payment_methods);
      this.panel1.Controls.Add(this.label2);
      this.panel1.Controls.Add(this.prices_textbox);
      this.panel1.Controls.Add(this.label1);
      this.panel1.Controls.Add(this.supported_anticheat_textbox);
      this.panel1.Controls.Add(this.status_label);
      this.panel1.Controls.Add(this.create_list_button);
      this.panel1.Controls.Add(this.menu_file_path);
      this.panel1.Controls.Add(this.import_menu_button);
      this.panel1.Dock = System.Windows.Forms.DockStyle.Top;
      this.panel1.Location = new System.Drawing.Point(0, 0);
      this.panel1.Name = "panel1";
      this.panel1.Size = new System.Drawing.Size(736, 177);
      this.panel1.TabIndex = 0;
      // 
      // status_label
      // 
      this.status_label.AutoSize = true;
      this.status_label.Location = new System.Drawing.Point(123, 40);
      this.status_label.Name = "status_label";
      this.status_label.Size = new System.Drawing.Size(0, 13);
      this.status_label.TabIndex = 3;
      // 
      // create_list_button
      // 
      this.create_list_button.Location = new System.Drawing.Point(3, 31);
      this.create_list_button.Name = "create_list_button";
      this.create_list_button.Size = new System.Drawing.Size(113, 23);
      this.create_list_button.TabIndex = 2;
      this.create_list_button.Text = "Create list";
      this.create_list_button.UseVisualStyleBackColor = true;
      this.create_list_button.Click += new System.EventHandler(this.create_list_button_Click);
      // 
      // menu_file_path
      // 
      this.menu_file_path.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
      this.menu_file_path.Location = new System.Drawing.Point(123, 4);
      this.menu_file_path.Name = "menu_file_path";
      this.menu_file_path.ReadOnly = true;
      this.menu_file_path.Size = new System.Drawing.Size(601, 20);
      this.menu_file_path.TabIndex = 1;
      this.menu_file_path.Text = "M:\\RIJIN\\codebase_main\\src\\cheats\\team_fortress2\\gui\\gcs_menu.h";
      // 
      // import_menu_button
      // 
      this.import_menu_button.Location = new System.Drawing.Point(3, 3);
      this.import_menu_button.Name = "import_menu_button";
      this.import_menu_button.Size = new System.Drawing.Size(113, 23);
      this.import_menu_button.TabIndex = 0;
      this.import_menu_button.Text = "Import gcs_menu";
      this.import_menu_button.UseVisualStyleBackColor = true;
      this.import_menu_button.Click += new System.EventHandler(this.import_menu_button_Click);
      // 
      // feature_list
      // 
      this.feature_list.BorderStyle = System.Windows.Forms.BorderStyle.FixedSingle;
      this.feature_list.Dock = System.Windows.Forms.DockStyle.Fill;
      this.feature_list.Location = new System.Drawing.Point(0, 177);
      this.feature_list.Name = "feature_list";
      this.feature_list.Size = new System.Drawing.Size(736, 270);
      this.feature_list.TabIndex = 1;
      this.feature_list.Text = "";
      // 
      // file_dialog
      // 
      this.file_dialog.FileName = "RijiN Menu";
      this.file_dialog.Filter = "C Header files|*.h";
      // 
      // supported_anticheat_textbox
      // 
      this.supported_anticheat_textbox.Location = new System.Drawing.Point(12, 82);
      this.supported_anticheat_textbox.Multiline = true;
      this.supported_anticheat_textbox.Name = "supported_anticheat_textbox";
      this.supported_anticheat_textbox.ScrollBars = System.Windows.Forms.ScrollBars.Both;
      this.supported_anticheat_textbox.Size = new System.Drawing.Size(181, 89);
      this.supported_anticheat_textbox.TabIndex = 4;
      this.supported_anticheat_textbox.Text = "Valve anti-cheat\r\nSMAC\r\nSTAC";
      // 
      // label1
      // 
      this.label1.AutoSize = true;
      this.label1.Location = new System.Drawing.Point(9, 66);
      this.label1.Name = "label1";
      this.label1.Size = new System.Drawing.Size(151, 13);
      this.label1.TabIndex = 5;
      this.label1.Text = "Supported anti-cheat(s):";
      // 
      // prices_textbox
      // 
      this.prices_textbox.Location = new System.Drawing.Point(199, 82);
      this.prices_textbox.Multiline = true;
      this.prices_textbox.Name = "prices_textbox";
      this.prices_textbox.ScrollBars = System.Windows.Forms.ScrollBars.Both;
      this.prices_textbox.Size = new System.Drawing.Size(181, 89);
      this.prices_textbox.TabIndex = 6;
      this.prices_textbox.Text = "£5.99 - 7 days\r\n£9.99 - 30 days\r\n£24.99 - 90 days\r\n£44.99 - 180 days\r\n£74.99 - 36" +
    "5 days";
      // 
      // label2
      // 
      this.label2.AutoSize = true;
      this.label2.Location = new System.Drawing.Point(196, 66);
      this.label2.Name = "label2";
      this.label2.Size = new System.Drawing.Size(151, 13);
      this.label2.TabIndex = 7;
      this.label2.Text = "Subscriptions and prices";
      // 
      // label3
      // 
      this.label3.AutoSize = true;
      this.label3.Location = new System.Drawing.Point(383, 66);
      this.label3.Name = "label3";
      this.label3.Size = new System.Drawing.Size(97, 13);
      this.label3.TabIndex = 9;
      this.label3.Text = "Payment methods";
      // 
      // payment_methods
      // 
      this.payment_methods.Location = new System.Drawing.Point(386, 82);
      this.payment_methods.Multiline = true;
      this.payment_methods.Name = "payment_methods";
      this.payment_methods.ScrollBars = System.Windows.Forms.ScrollBars.Both;
      this.payment_methods.Size = new System.Drawing.Size(181, 89);
      this.payment_methods.TabIndex = 8;
      this.payment_methods.Text = "Accepting Credit Card, Debit Card, and PayPal.";
      // 
      // main_form
      // 
      this.AutoScaleDimensions = new System.Drawing.SizeF(6F, 13F);
      this.AutoScaleMode = System.Windows.Forms.AutoScaleMode.Font;
      this.ClientSize = new System.Drawing.Size(736, 447);
      this.Controls.Add(this.feature_list);
      this.Controls.Add(this.panel1);
      this.Font = new System.Drawing.Font("Consolas", 8.25F, System.Drawing.FontStyle.Regular, System.Drawing.GraphicsUnit.Point, ((byte)(0)));
      this.MaximizeBox = false;
      this.Name = "main_form";
      this.ShowIcon = false;
      this.StartPosition = System.Windows.Forms.FormStartPosition.CenterScreen;
      this.Text = "RijiN Feature-list Creator";
      this.Load += new System.EventHandler(this.main_form_Load);
      this.panel1.ResumeLayout(false);
      this.panel1.PerformLayout();
      this.ResumeLayout(false);

    }

    #endregion

    private System.Windows.Forms.Panel panel1;
    private System.Windows.Forms.RichTextBox feature_list;
    private System.Windows.Forms.TextBox menu_file_path;
    private System.Windows.Forms.Button import_menu_button;
    private System.Windows.Forms.OpenFileDialog file_dialog;
    private System.Windows.Forms.Button create_list_button;
    private System.Windows.Forms.Label status_label;
    private System.Windows.Forms.Label label3;
    private System.Windows.Forms.TextBox payment_methods;
    private System.Windows.Forms.Label label2;
    private System.Windows.Forms.TextBox prices_textbox;
    private System.Windows.Forms.Label label1;
    private System.Windows.Forms.TextBox supported_anticheat_textbox;
  }
}

