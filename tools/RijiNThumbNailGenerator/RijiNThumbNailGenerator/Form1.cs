using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Drawing.Imaging;
using System.Drawing.Drawing2D;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Windows.Forms;

namespace RijiNThumbNailGenerator {
  public partial class MainForm : Form {
    public MainForm() {
      InitializeComponent();
    }

    int max_width = 1280;
    int max_height = 720;

    private void create_image() {
      try {
        Bitmap logo = new Bitmap("logo_resized.png");
        Bitmap bmp = new Bitmap(max_width, max_height);
        Graphics g = Graphics.FromImage(bmp);
        g.SmoothingMode = SmoothingMode.HighQuality;
        g.InterpolationMode = InterpolationMode.HighQualityBilinear;
        g.TextRenderingHint = System.Drawing.Text.TextRenderingHint.AntiAlias;
        g.CompositingQuality = CompositingQuality.HighQuality;
        g.PixelOffsetMode = PixelOffsetMode.HighQuality;
      
        string text = textBox1.Text;

        float size = 72f;
        if (text.Length >= 26)
          size = 24f;
        else if (text.Length >= 18)
          size = 48f;

        Font f = new Font("Arial Black", size, FontStyle.Regular);
        SolidBrush background_clr = new SolidBrush(Color.FromArgb(255, 17, 38, 69));
        SolidBrush green_box_clr = new SolidBrush(Color.FromArgb(255, 24, 68, 79));
        SolidBrush txt_clr = new SolidBrush(Color.FromArgb(255, 67, 217, 188));
        g.FillRectangle(background_clr, -1, -1, max_width + 1, max_height + 1);

        text = text.ToUpper();
        var result = g.MeasureString(text, f);

        int offset = 96;
        int box_x = (Width / 2) - (int)(result.Width / 2);
        int box_y = (Height / 2) - (int)(result.Height / 2);
        box_y += offset;

        g.FillRectangle(green_box_clr, box_x, box_y, result.Width, result.Height);
        g.DrawString(text, f, txt_clr, box_x, box_y);
        g.DrawImageUnscaledAndClipped(logo, new Rectangle((int)(box_x + result.Width / 2) - (int)(logo.Width / 2), (box_y - offset) - (int)(result.Height / 2) - offset, logo.Width, logo.Height));

        BackgroundImage = bmp;
        bmp.Save("exported_thumbnail.png");
      }
      catch {
        Text = "ERROR";
      }
    }
    private void Form1_Load(object sender, EventArgs e) {
      create_image();
    }

    private void button1_Click(object sender, EventArgs e) {
      create_image();
    }

    private void textBox1_TextChanged(object sender, EventArgs e) {
      create_image();
    }
  }
}
