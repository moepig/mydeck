namespace MyDeck.Logging;

public sealed class EventLog
{
    public record Entry(DateTime Time, string Kind, string Detail);

    private readonly List<Entry> _entries = [];
    private readonly object _lock = new();

    public event Action? Changed;

    public void Add(Entry entry)
    {
        lock (_lock) _entries.Add(entry);
        Changed?.Invoke();
    }

    public IReadOnlyList<Entry> Snapshot()
    {
        lock (_lock) return [.._entries];
    }

    public void Clear()
    {
        lock (_lock) _entries.Clear();
        Changed?.Invoke();
    }
}
