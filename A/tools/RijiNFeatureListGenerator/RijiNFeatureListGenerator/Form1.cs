using System;
using System.Collections.Generic;
using System.ComponentModel;
using System.Data;
using System.Drawing;
using System.Linq;
using System.Text;
using System.Threading.Tasks;
using System.Threading;
using System.Windows.Forms;
using System.IO;
using System.Windows.Forms.VisualStyles;
using System.Runtime.InteropServices;
using System.Security.Principal;

namespace RijiNFeatureListGenerator {
  public partial class main_form : Form {
    public main_form() {
      InitializeComponent();
    }


    /*  
     *  NOTICE TO READER
     * 
     * You can use '// HINT-HEADER: THIS IS A HEADER' in the event you don't have code written for a certain feature.
     * 
     * */

    string[] header_types = { " create_" };
    string[] object_types = { "gcs::toggle(", "gcs::dropdown(", "gcs::slider(", "gcs::label(", "// HINT-HEADER: " };
    string header_text = "\n●";
    string hint_header_text = "\n▼";
    string group_header_text = "\n*";
    string feature_text = "  →";

    // Keep track what was already added.
    List<string> feature_names = new List<string>();

    private void main_form_Load(object sender, EventArgs e) {
      CheckForIllegalCrossThreadCalls = false;
    }

    private void import_menu_button_Click(object sender, EventArgs e) {
      file_dialog.ShowDialog();
      if (File.Exists(file_dialog.FileName)) {
        menu_file_path.Text = file_dialog.FileName;
      }
    }

    private void compute_list() {
      if (!File.Exists(menu_file_path.Text)) {
        MessageBox.Show("Failed to find menu file", "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
        return;
      }

      feature_names.Clear();
      var sr = new StreamReader(menu_file_path.Text);
      string file_contents = sr.ReadToEnd();
      sr.Close();
      sr.Dispose();

      clean_string_for_parse(ref file_contents);

      string buffer = "";
      int current_pos = file_contents.IndexOf(header_types[0]);

      //We assume the first thing in the gcs_menu file is a header.
      compute_func_header(current_pos, file_contents, ref buffer);
      while (current_pos < file_contents.Length) {

        int next_toggle = file_contents.IndexOf(object_types[0], current_pos + 1);
        int next_dropdown = file_contents.IndexOf(object_types[1], current_pos + 1);
        int next_slider = file_contents.IndexOf(object_types[2], current_pos + 1);
        int next_label = file_contents.IndexOf(object_types[3], current_pos + 1);
        int next_hint = file_contents.IndexOf(object_types[4], current_pos + 1);

        int next_func_header = file_contents.IndexOf(header_types[0], current_pos + 1);

        int[] positions = { next_func_header, next_toggle, next_dropdown, next_slider, next_label, next_hint };
        int best_c = -1;

        for (int c = 0; c < positions.Length; c++) {
          if (positions[c] == -1)
            continue;

          bool pass = true;
          for (int i = 0; i < positions.Length; i++) {
            if (positions[c] > positions[i] && positions[i] != -1)
              pass = false;
          }

          if (pass) {
            current_pos = positions[c];
            best_c = c;
            break;
          }
        }

        if (best_c == 0) {
          compute_func_header(current_pos, file_contents, ref buffer);
        }
        else if (best_c == 1)
          compute_toggle(current_pos, file_contents, ref buffer);
        else if (best_c == 2)
          compute_dropdown(current_pos, file_contents, ref buffer);
        else if (best_c == 3)
          compute_slider(current_pos, file_contents, ref buffer);
        else if (best_c == 4)
          compute_label(current_pos, file_contents, ref buffer);
        else if (best_c == 5)
          compute_hint_header(current_pos, file_contents, ref buffer);
        else {
          break;
        }
      }

      create_list_button.Enabled = true;

      buffer = buffer.Replace("Esp", "ESP").Replace("Hvh", "Hack vs Hack");
      feature_list.Text += "Supported anti-cheat(s):\n" + supported_anticheat_textbox.Text + "\n\n\n\n";
      feature_list.Text += "Prices\n" + prices_textbox.Text + "\n";
      feature_list.Text += payment_methods.Text + "\n\n<LINK>\n\n";
      feature_list.Text += buffer;

      if(current_pos != -1 || current_pos >= file_contents.Length) {
        MessageBox.Show("List should've been generated.\nReview list for formatting issues and features that aren't supposed to be public.", "Notice", MessageBoxButtons.OK, MessageBoxIcon.Information);
      }
    }
    private void create_list_button_Click(object sender, EventArgs e) {
      try {
        feature_list.Text = "";
        create_list_button.Enabled = false;
        new Thread(compute_list).Start();
      }
      catch { }
    }
    private bool compute_hint_header(int current_index, string file_buffer, ref string buffer) {
      if (current_index == -1)
        return false;

      string name = "";
      for (int i = current_index; i < file_buffer.Length; i++) {
        if (file_buffer[i] == '\n')
          break;

        name += file_buffer[i];
      }

      name = name.Replace("// HINT-HEADER: ", "");
      name = name.Replace(".", "").Replace("!", "");
      name = name.TrimStart().TrimEnd();

      if (!should_write_to_list(name))
        return true;

      name = name.TrimStart();
      add_to_known_list(name);
      buffer += hint_header_text + " ";
      buffer += trim_and_upper_name(name, false);
      buffer += "\n";
      return true;
    }
    private bool compute_func_header(int current_index, string file_buffer, ref string buffer) {
      if (current_index == -1)
        return false;

      string name = "";
      for (int i = current_index; i < file_buffer.Length; i++) {
        if (file_buffer[i] == '(')
          break;

        name += file_buffer[i];
      }

      name = name.Replace("void ", "");
      name = name.Replace("create_", "").Replace("_layer", "");
      name = name.Replace("_", " ");
      name = name.TrimStart().TrimEnd();

      if (!should_write_to_list(name))
        return true;

      name = name.TrimStart();
      add_to_known_list(name);
      buffer += header_text + " ";
      buffer += trim_and_upper_name(name, false);
      buffer += "\n\n";
      return true;
    }
    private bool compute_toggle(int current_index, string file_buffer, ref string buffer) {
      if (current_index == -1)
        return false;

      string name = "";
      bool should_write = false;
      for (int i = current_index; i < file_buffer.Length; i++) {
        if (file_buffer[i] == '"')
          continue;

        if (file_buffer[i] == ',') {
          if (should_write)
            break;

          should_write = true;
          continue;
        }

        if (should_write)
          name += file_buffer[i];
      }

      if (!should_write_to_list(name))
        return true;

      name = name.TrimStart();
      add_to_known_list(name);
      buffer += feature_text + " ";
      buffer += trim_and_upper_name(name);
      buffer += "\n";
      return true;
    }
    private bool compute_label(int current_index, string file_buffer, ref string buffer) {
      if (current_index == -1)
        return false;

      string name = "";
      bool should_write = false;
      for (int i = current_index; i < file_buffer.Length; i++) {
        if (file_buffer[i] == '"') {
          if (should_write)
            break;

          should_write = true;
          continue;
        }

        if (should_write)
          name += file_buffer[i];
      }

      if (!should_write_to_list(name))
        return true;

      int colour_index = file_buffer.IndexOf("set_output_colour_ptr", current_index + 1);
      int hotkey_index = file_buffer.IndexOf("gcs_component_hotkey", current_index + 1);
      int end_pos = file_buffer.IndexOf(";", current_index + 1);

      name = name.TrimStart();
      add_to_known_list(name);
      buffer += feature_text + " ";
      buffer += trim_and_upper_name(name);
      if (colour_index < end_pos && colour_index != -1)
        buffer += " (Color Picker)";
      else if (hotkey_index < end_pos && hotkey_index != -1)
        buffer += " (Hotkey)";

      buffer += "\n";
      return true;
    }
    private bool compute_dropdown(int current_index, string file_buffer, ref string buffer) {
      if (current_index == -1)
        return false;

      string name = "";
      bool should_write = false;
      for (int i = current_index; i < file_buffer.Length; i++) {
        if (file_buffer[i] == '"')
          continue;

        if (file_buffer[i] == ',') {
          if (should_write)
            break;

          should_write = true;
          continue;
        }

        if (should_write)
          name += file_buffer[i];
      }

      if (!should_write_to_list(name))
        return true;

      List<string> selection_options = new List<string>();
      int end_pos = file_buffer.IndexOf(';', current_index + 1);
      int start_pos = file_buffer.IndexOf("add_option(", current_index + 1);
      int first_start_pos = start_pos;
      while (start_pos != -1 && start_pos < end_pos && start_pos >= first_start_pos) {

        string entry = "";
        bool write_entry = false;
        for (int i = start_pos; i < end_pos; i++) {

          if (file_buffer[i] == '"') {
            if (write_entry)
              break;

            write_entry = true;
            continue;
          }

          if (write_entry)
            entry += file_buffer[i];
        }

        if (entry.Length > 0)
          selection_options.Add(entry);

        start_pos = file_buffer.IndexOf("add_option(", start_pos + 1);
        if (start_pos == -1 || start_pos > end_pos || start_pos < first_start_pos)
          break;
      }

      name = name.TrimStart();
      add_to_known_list(name);
      buffer += feature_text + " ";
      buffer += trim_and_upper_name(name) + ": ( ";
      for (int i = 0; i < selection_options.Count; i++) {
        if (i == selection_options.Count - 1)
          buffer += selection_options[i] + " )";
        else
          buffer += selection_options[i] + ", ";
      }

      buffer += "\n";
      return true;
    }
    private bool compute_slider(int current_index, string file_buffer, ref string buffer) {
      if (current_index == -1)
        return false;

      string name = "";
      bool should_write = false;
      for (int i = current_index; i < file_buffer.Length; i++) {
        if (file_buffer[i] == '"')
          continue;

        if (file_buffer[i] == ',') {
          if (should_write)
            break;

          should_write = true;
          continue;
        }

        if (should_write)
          name += file_buffer[i];
      }

      name = name.TrimStart();
      if (!should_write_to_list(name))
        return true;

      string modifier_text = "";
      bool should_write_mod = false;

      string min_value = "";
      string max_value = "";

      // Detect the 'modifier text'
      {
        int start_pos = file_buffer.IndexOf(name + "\",", current_index + 1);
        int end_pos = file_buffer.IndexOf(";", current_index + 1);
        int quote_count = 1;

        // How many quotes are there in here?
        for (int i = start_pos; i < end_pos; i++) {
          if (file_buffer[i] == '"')
            quote_count++;
        }

        if (start_pos != -1) {
          start_pos += name.Length + 2;

          if (quote_count > 2) {
            // Find the modifier text.
            {
              for (int i = start_pos; i < file_buffer.Length; i++) {
                if (file_buffer[i] == '"') {
                  if (should_write_mod)
                    break;

                  should_write_mod = true;
                  continue;
                }

                if (should_write_mod)
                  modifier_text += file_buffer[i];
              }
            }

            // Find range.
            start_pos = file_buffer.IndexOf(",", start_pos + 1);
          }

          if (start_pos != -1) {
            start_pos += 1;

            // Min value.
            {
              for (int i = start_pos; i < file_buffer.Length; i++) {
                if (file_buffer[i] == ',')
                  break;

                min_value += file_buffer[i];
              }

              min_value = min_value.Trim();
              if (!should_write_to_list(min_value)) {
                MessageBox.Show("Failed to parse slider parameters(Min Value): " + name + " -> " + min_value + "!\nAre you using a variable to control the min size?", "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
                return false;
              }

              if (modifier_text.Length >= 1 && modifier_text.Length <= 2)
                min_value += modifier_text;
            }

            // Max value.
            {
              start_pos = file_buffer.IndexOf(",", start_pos + 1);
              if (start_pos != -1) {
                start_pos++;

                for (int i = start_pos; i < file_buffer.Length; i++) {
                  if (file_buffer[i] == ',')
                    break;

                  max_value += file_buffer[i];
                }

                max_value = max_value.Trim();
                if (!should_write_to_list(max_value)) {
                  MessageBox.Show("Failed to parse slider parameters(Max Value): " + name + " -> \"" + max_value + "\"!\n\nAre you using a variable to control the max size?", "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
                  return false;
                }

                if (modifier_text.Length >= 1 && modifier_text.Length <= 2)
                  max_value += modifier_text;
              }
            }

          }
        }
        else {
          MessageBox.Show("Failed to parse slider parameters: " + name + "!", "Error", MessageBoxButtons.OK, MessageBoxIcon.Error);
          return false;
        }
      }


      add_to_known_list(name);
      buffer += feature_text + " ";
      buffer += trim_and_upper_name(name) + ": ";

      buffer += "( ";
      buffer += min_value + " to " + max_value + " )";

      buffer += "\n";
      return true;
    }
    private void clean_string_for_parse(ref string str) {
      if (str == null)
        return;

      if (str.Length == 0)
        return;

      str = str.Replace("WXOR(L", "").Replace("\"),", "\",");
      str = str.Replace("L\"\"", "\"\"");
      while (str.Contains("  "))
        str = str.Replace("  ", " ");
    }
    private bool should_write_to_list(string name) {
      if (name == null || name.Length == 0)
        return false;

      name = name.ToLower();

      // Never add these.
      if (name.Contains("component") || name == "create_aimbot" || name.Contains("crash") || name.Contains("lagger"))
        return false;

      // Detect parse errors.
      if (name.Contains("=") || name.Contains(";") || name.Contains("->") || name.Contains("gcs_") || name.Contains("=")) {
        MessageBox.Show("Parse error: " + name, "ERROR", MessageBoxButtons.OK, MessageBoxIcon.Error);
        return false;
      }

      if (name.Contains("xor(l")) {
        MessageBox.Show("Possible programmer mistake: " + name, "ERROR", MessageBoxButtons.OK, MessageBoxIcon.Error);
        return false;
      }

      if (name.Contains("key") || name.Contains("fov") || name.Contains("flags") || name.Contains("overlay") || name.Contains("team") || name.Contains("enemy") || name.Contains("pack") ||
        name.Contains("boss") || name.Contains("book") || name.Contains("power") || name.Contains("no ") || name.Contains("money") || name.Contains("cash") || name.Contains("bomb")
        || name.Contains("lunch"))
        return true;

      foreach (var i in feature_names) {
        if (name == i)
          return false;
      }

      return true;
    }

    private string trim_and_upper_name(string name, bool only_once = true) {
      if (name == null || name.Length == 0)
        return "";

      string b = "";

      name = name.TrimStart().TrimEnd();
      bool force_upper = false;
      for (int i = 0; i < name.Length; i++) {
        if (name[i] == ' ')
          force_upper = true;

        if (i == 0 || force_upper && name[i] != ' ' && !only_once) {
          b += name[i].ToString().ToUpper();
          force_upper = false;
        }
        else
          b += name[i];
      }

      return b;
    }

    private void add_to_known_list(string name) {
      if (!should_write_to_list(name))
        return;

      name = name.ToLower();
      feature_names.Add(name);
    }
  }
}
