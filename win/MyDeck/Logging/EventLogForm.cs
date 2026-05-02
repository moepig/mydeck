namespace MyDeck.Logging;

public sealed class EventLogForm : Form
{
    private readonly EventLog _log;
    private readonly ListView _list;

    public EventLogForm(EventLog log)
    {
        _log = log;
        _list = BuildList();
        BuildLayout();
        Reload();
        _log.Changed += OnLogChanged;
    }

    private static ListView BuildList()
    {
        var lv = new ListView
        {
            Dock          = DockStyle.Fill,
            View          = View.Details,
            FullRowSelect = true,
            GridLines     = true,
            Font          = new Font("Consolas", 9f),
        };
        lv.Columns.Add("時刻", 100);
        lv.Columns.Add("種別", 70);
        lv.Columns.Add("詳細", 700);
        return lv;
    }

    private void BuildLayout()
    {
        Text            = "MyDeck イベントログ";
        Size            = new Size(920, 520);
        MinimumSize     = new Size(600, 300);
        StartPosition   = FormStartPosition.CenterScreen;
        FormBorderStyle = FormBorderStyle.Sizable;

        var bottom    = new Panel { Dock = DockStyle.Bottom, Height = 40 };
        var btnClear  = new Button { Text = "クリア", Width = 88, Height = 26, Top = 7, Left = 8 };
        btnClear.Click += (_, _) => _log.Clear();
        bottom.Controls.Add(btnClear);

        Controls.Add(_list);
        Controls.Add(bottom);
    }

    private void Reload()
    {
        _list.BeginUpdate();
        _list.Items.Clear();
        foreach (var e in _log.Snapshot())
            _list.Items.Add(ToItem(e));
        _list.EndUpdate();
        ScrollToBottom();
    }

    private static ListViewItem ToItem(EventLog.Entry e)
    {
        var item = new ListViewItem(e.Time.ToString("HH:mm:ss.fff"));
        item.SubItems.Add(e.Kind);
        item.SubItems.Add(e.Detail);
        return item;
    }

    private void ScrollToBottom()
    {
        if (_list.Items.Count > 0)
            _list.EnsureVisible(_list.Items.Count - 1);
    }

    private void OnLogChanged()
    {
        if (IsDisposed) return;
        if (InvokeRequired) { BeginInvoke(OnLogChanged); return; }

        var all = _log.Snapshot();
        _list.BeginUpdate();
        if (_list.Items.Count > all.Count)
        {
            // クリア後は全再描画
            _list.Items.Clear();
            foreach (var e in all)
                _list.Items.Add(ToItem(e));
        }
        else
        {
            // 差分追記
            while (_list.Items.Count < all.Count)
                _list.Items.Add(ToItem(all[_list.Items.Count]));
        }
        _list.EndUpdate();
        ScrollToBottom();
    }

    protected override void OnFormClosed(FormClosedEventArgs e)
    {
        _log.Changed -= OnLogChanged;
        base.OnFormClosed(e);
    }
}
