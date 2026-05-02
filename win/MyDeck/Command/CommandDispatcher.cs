using MyDeck.Config;
using MyDeck.Hid;

namespace MyDeck.Command;

public sealed class CommandDispatcher
{
    private readonly IMyDeckCommandExecutor _executor;
    // コンストラクタ時に (buttonId, eventName) → actions を事前構築して O(1) ディスパッチ
    private readonly Dictionary<(byte, string), List<ActionConfig>> _map;

    public CommandDispatcher(IMyDeckCommandExecutor executor, IReadOnlyList<ButtonConfig> buttons)
    {
        _executor = executor;
        _map = new Dictionary<(byte, string), List<ActionConfig>>();
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

        if (_map.TryGetValue((report.ButtonId, eventName), out var actions))
            foreach (var action in actions)
                _executor.Execute(action.Executable, action.Arguments);
    }
}
