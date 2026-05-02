using MyDeck.Logging;
using NUnit.Framework;

namespace MyDeck.Tests.Logging;

[TestFixture]
public class EventLogTests
{
    private EventLog _log = null!;

    [SetUp]
    public void SetUp() => _log = new EventLog();

    [Test]
    public void Add_Entry_AppearsInSnapshot()
    {
        _log.Add(new EventLog.Entry(DateTime.Now, "種別", "詳細"));

        Assert.That(_log.Snapshot(), Has.Count.EqualTo(1));
    }

    [Test]
    public void Snapshot_MultipleEntries_ReturnsAll()
    {
        _log.Add(new EventLog.Entry(DateTime.Now, "A", "1"));
        _log.Add(new EventLog.Entry(DateTime.Now, "B", "2"));

        Assert.That(_log.Snapshot(), Has.Count.EqualTo(2));
    }

    [Test]
    public void Clear_AfterAdding_SnapshotIsEmpty()
    {
        _log.Add(new EventLog.Entry(DateTime.Now, "A", "1"));
        _log.Clear();

        Assert.That(_log.Snapshot(), Is.Empty);
    }

    [Test]
    public void Add_FiresChangedEvent()
    {
        bool fired = false;
        _log.Changed += () => fired = true;

        _log.Add(new EventLog.Entry(DateTime.Now, "A", "1"));

        Assert.That(fired, Is.True);
    }

    [Test]
    public void Clear_FiresChangedEvent()
    {
        bool fired = false;
        _log.Changed += () => fired = true;

        _log.Clear();

        Assert.That(fired, Is.True);
    }

    [Test]
    public void Snapshot_IsImmutableCopy()
    {
        _log.Add(new EventLog.Entry(DateTime.Now, "A", "1"));
        var snap = _log.Snapshot();

        _log.Add(new EventLog.Entry(DateTime.Now, "B", "2"));

        Assert.That(snap,          Has.Count.EqualTo(1));
        Assert.That(_log.Snapshot(), Has.Count.EqualTo(2));
    }

    [Test]
    public void Add_FromMultipleThreads_AllEntriesPresent()
    {
        const int threadCount      = 10;
        const int entriesPerThread = 100;

        var threads = Enumerable.Range(0, threadCount)
            .Select(_ => new Thread(() =>
            {
                for (int i = 0; i < entriesPerThread; i++)
                    _log.Add(new EventLog.Entry(DateTime.Now, "T", i.ToString()));
            }))
            .ToList();

        threads.ForEach(t => t.Start());
        threads.ForEach(t => t.Join());

        Assert.That(_log.Snapshot(), Has.Count.EqualTo(threadCount * entriesPerThread));
    }
}
