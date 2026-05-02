using MyDeck.Config;
using MyDeck.Hid;
using MyDeck.Logging;

namespace MyDeck.Command;

public sealed class CommandDispatcher
{
    private readonly IMyDeckCommandExecutor _executor;
    private readonly EventLog _log;
    private readonly Dictionary<(byte, string), List<ActionConfig>> _map;
    private readonly Dictionary<byte, string> _labels;

    public CommandDispatcher(IMyDeckCommandExecutor executor, EventLog log, IReadOnlyList<ButtonConfig> buttons)
    {
        _executor = executor;
        _log      = log;
        _map      = [];
        _labels   = buttons.ToDictionary(b => b.ButtonId, b => b.Label);

        foreach (var btn in buttons)
            foreach (var action in btn.Actions)
            {
                var key = (btn.ButtonId, action.Event);
                if (!_map.TryGetValue(key, out var list))
                    _map[key] = list = [];
                list.Add(action);
            }
    }

    public void Dispatch(InputReport report)
    {
        var eventName = report.Event switch
        {
            ButtonEventType.Press   => "press",
            ButtonEventType.Release => "release",
            ButtonEventType.Hold    => "hold",
            _                       => null,
        };
        if (eventName is null) return;

        var label = _labels.TryGetValue(report.ButtonId, out var l) && !string.IsNullOrEmpty(l)
            ? $" [{l}]"
            : "";
        _log.Add(new EventLog.Entry(DateTime.Now, "ボタン", $"ID={report.ButtonId}{label} {eventName}"));

        if (_map.TryGetValue((report.ButtonId, eventName), out var actions))
            foreach (var action in actions)
                _executor.Execute(action.Executable, action.Arguments);
    }
}
