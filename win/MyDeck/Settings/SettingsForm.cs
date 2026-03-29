using System.Diagnostics;
using MyDeck.Config;

namespace MyDeck.Settings;

public sealed class SettingsForm : Form
{
    private readonly string _configPath;
    private AppConfig _current;
    private readonly DataGridView _grid;

    /// <summary>保存ボタン押下後、新しい設定を通知する。</summary>
    public event Action<AppConfig>? ConfigSaved;

    public SettingsForm(AppConfig config, string configPath)
    {
        _current    = config;
        _configPath = configPath;
        _grid       = BuildGrid();
        BuildLayout();
        LoadGrid();
    }

    // ---- UI 構築 -----------------------------------------------

    private DataGridView BuildGrid()
    {
        var eventCol = new DataGridViewComboBoxColumn
        {
            Name        = "Event",
            HeaderText  = "イベント",
            FillWeight  = 10,
        };
        eventCol.Items.AddRange("press", "hold", "release");

        var g = new DataGridView
        {
            Dock                  = DockStyle.Fill,
            AllowUserToAddRows    = false,
            AllowUserToDeleteRows = false,
            AutoSizeColumnsMode   = DataGridViewAutoSizeColumnsMode.Fill,
            SelectionMode         = DataGridViewSelectionMode.FullRowSelect,
            MultiSelect           = false,
            RowHeadersVisible     = false,
            EditMode              = DataGridViewEditMode.EditOnEnter,
        };
        g.DataError += (_, e) => e.ThrowException = false;

        g.Columns.Add(new DataGridViewTextBoxColumn { Name = "ButtonId",   HeaderText = "ID",       FillWeight = 5  });
        g.Columns.Add(new DataGridViewTextBoxColumn { Name = "Label",      HeaderText = "ラベル",   FillWeight = 18 });
        g.Columns.Add(eventCol);
        g.Columns.Add(new DataGridViewTextBoxColumn { Name = "Executable", HeaderText = "実行ファイル", FillWeight = 42 });
        g.Columns.Add(new DataGridViewTextBoxColumn { Name = "Arguments",  HeaderText = "引数",     FillWeight = 25 });

        return g;
    }

    private void BuildLayout()
    {
        Text            = "MyDeck 設定";
        Size            = new Size(900, 520);
        MinimumSize     = new Size(660, 380);
        StartPosition   = FormStartPosition.CenterScreen;
        FormBorderStyle = FormBorderStyle.Sizable;

        var bottom = new Panel { Dock = DockStyle.Bottom, Height = 44 };

        // 左寄せボタン群
        int x = 8;
        Button Left(string text, EventHandler handler)
        {
            var b = new Button { Text = text, Left = x, Top = 8, Width = 92, Height = 28 };
            b.Click += handler;
            bottom.Controls.Add(b);
            x += 100;
            return b;
        }
        Left("追加",      OnAdd);
        Left("削除",      OnDelete);
        Left("テスト実行", OnTest);

        // 右寄せボタン群
        var btnClose = new Button { Text = "閉じる", Width = 92, Height = 28, Top = 8, Anchor = AnchorStyles.Right | AnchorStyles.Top };
        var btnSave  = new Button { Text = "保存",   Width = 92, Height = 28, Top = 8, Anchor = AnchorStyles.Right | AnchorStyles.Top };
        btnClose.Click += (_, _) => Close();
        btnSave.Click  += OnSave;
        bottom.Controls.AddRange([btnSave, btnClose]);
        bottom.SizeChanged += (_, _) =>
        {
            btnClose.Left = bottom.Width - 100;
            btnSave.Left  = bottom.Width - 200;
        };

        Controls.Add(_grid);
        Controls.Add(bottom);
    }

    // ---- データ操作 -------------------------------------------

    private void LoadGrid()
    {
        _grid.Rows.Clear();
        foreach (var btn in _current.Buttons)
            foreach (var action in btn.Actions)
                _grid.Rows.Add(btn.ButtonId, btn.Label, action.Event, action.Executable, action.Arguments);
    }

    private void OnAdd(object? s, EventArgs e)
    {
        _grid.Rows.Add("1", "", "press", "", "");
        int idx = _grid.Rows.Count - 1;
        _grid.ClearSelection();
        _grid.Rows[idx].Selected = true;
        _grid.CurrentCell = _grid.Rows[idx].Cells["ButtonId"];
        _grid.BeginEdit(true);
    }

    private void OnDelete(object? s, EventArgs e)
    {
        if (_grid.SelectedRows.Count == 0) return;
        _grid.Rows.RemoveAt(_grid.SelectedRows[0].Index);
    }

    private void OnTest(object? s, EventArgs e)
    {
        if (_grid.SelectedRows.Count == 0) return;
        var row  = _grid.SelectedRows[0];
        var exe  = row.Cells["Executable"].Value?.ToString() ?? "";
        var args = row.Cells["Arguments"].Value?.ToString()  ?? "";

        if (string.IsNullOrWhiteSpace(exe))
        {
            MessageBox.Show("実行ファイルが設定されていません。", "MyDeck",
                            MessageBoxButtons.OK, MessageBoxIcon.Warning);
            return;
        }
        try
        {
            Process.Start(new ProcessStartInfo
            {
                FileName        = exe,
                Arguments       = args,
                UseShellExecute = true,
            });
        }
        catch (Exception ex)
        {
            MessageBox.Show($"実行エラー:\n{ex.Message}", "MyDeck",
                            MessageBoxButtons.OK, MessageBoxIcon.Error);
        }
    }

    private void OnSave(object? s, EventArgs e)
    {
        _grid.CommitEdit(DataGridViewDataErrorContexts.Commit);

        // グリッドの各行を (buttonId → label + actions) に集約
        var groups = new Dictionary<byte, (string label, List<ActionConfig> actions)>();
        foreach (DataGridViewRow row in _grid.Rows)
        {
            if (!byte.TryParse(row.Cells["ButtonId"].Value?.ToString(), out byte id))
            {
                MessageBox.Show($"行 {row.Index + 1}: ID が不正です（1〜255 の整数を入力してください）。",
                                "MyDeck", MessageBoxButtons.OK, MessageBoxIcon.Warning);
                return;
            }
            var label = row.Cells["Label"].Value?.ToString()      ?? "";
            var ev    = row.Cells["Event"].Value?.ToString()       ?? "press";
            var exe   = row.Cells["Executable"].Value?.ToString()  ?? "";
            var args  = row.Cells["Arguments"].Value?.ToString()   ?? "";

            if (!groups.TryGetValue(id, out var entry))
                groups[id] = entry = (label, []);
            entry.actions.Add(new ActionConfig
            {
                Event      = ev,
                Type       = "process",
                Executable = exe,
                Arguments  = args,
            });
        }

        var newConfig = _current with
        {
            Buttons = groups
                .OrderBy(kv => kv.Key)
                .Select(kv => new ButtonConfig
                {
                    ButtonId = kv.Key,
                    Label    = kv.Value.label,
                    Actions  = kv.Value.actions,
                })
                .ToList(),
        };

        try
        {
            ConfigLoader.Save(newConfig, _configPath);
            _current = newConfig;
            ConfigSaved?.Invoke(newConfig);
            MessageBox.Show("保存しました。", "MyDeck", MessageBoxButtons.OK, MessageBoxIcon.Information);
        }
        catch (Exception ex)
        {
            MessageBox.Show($"保存エラー:\n{ex.Message}", "MyDeck",
                            MessageBoxButtons.OK, MessageBoxIcon.Error);
        }
    }
}
