using MyDeck;
using NUnit.Framework;

namespace MyDeck.Tests.SingleInstance;

[TestFixture]
public sealed class SingleInstanceGuardTests
{
    // テストごとにユニークな名前を使い、他のテストと干渉しない
    private static string UniqueName() => "MyDeck-Test-" + Guid.NewGuid();

    // ---- IsFirstInstance -------------------------------------------

    [Test]
    public void IsFirstInstance_WhenNoOtherInstance_ReturnsTrue()
    {
        using var guard = new SingleInstanceGuard(UniqueName(), UniqueName());

        Assert.That(guard.IsFirstInstance, Is.True);
    }

    [Test]
    public void IsFirstInstance_WhenOtherInstanceExists_ReturnsFalse()
    {
        var mutexName = UniqueName();
        var eventName = UniqueName();

        using var first  = new SingleInstanceGuard(mutexName, eventName);
        using var second = new SingleInstanceGuard(mutexName, eventName);

        Assert.That(second.IsFirstInstance, Is.False);
    }

    [Test]
    public void IsFirstInstance_AfterFirstInstanceDisposed_ReturnsTrue()
    {
        var mutexName = UniqueName();
        var eventName = UniqueName();

        var first = new SingleInstanceGuard(mutexName, eventName);
        first.Dispose();

        using var second = new SingleInstanceGuard(mutexName, eventName);

        Assert.That(second.IsFirstInstance, Is.True);
    }

    // ---- TrySignal -------------------------------------------------

    [Test]
    public void TrySignal_WhenEventExists_ReturnsTrue()
    {
        var eventName = UniqueName();
        using var ev  = new EventWaitHandle(false, EventResetMode.AutoReset, eventName);

        using var guard = new SingleInstanceGuard(UniqueName(), eventName);

        Assert.That(guard.TrySignal(), Is.True);
    }

    [Test]
    public void TrySignal_WhenEventNotExists_ReturnsFalse()
    {
        using var guard = new SingleInstanceGuard(UniqueName(), UniqueName());

        Assert.That(guard.TrySignal(), Is.False);
    }

    [Test]
    public void TrySignal_SetsEvent_WaitOneSucceeds()
    {
        var eventName = UniqueName();
        using var ev  = new EventWaitHandle(false, EventResetMode.AutoReset, eventName);

        using var guard = new SingleInstanceGuard(UniqueName(), eventName);
        guard.TrySignal();

        Assert.That(ev.WaitOne(millisecondsTimeout: 0), Is.True);
    }

    [Test]
    public void TrySignal_WithoutSignal_WaitOneReturnsFalse()
    {
        var eventName = UniqueName();
        using var ev  = new EventWaitHandle(false, EventResetMode.AutoReset, eventName);

        Assert.That(ev.WaitOne(millisecondsTimeout: 0), Is.False);
    }
}
