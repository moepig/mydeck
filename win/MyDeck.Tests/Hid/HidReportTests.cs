using MyDeck.Hid;
using NUnit.Framework;

namespace MyDeck.Tests.Hid;

[TestFixture]
public class HidReportTests
{
    [Test]
    public void TryParse_ValidPressReport_ReturnsReport()
    {
        var data = new byte[8];
        data[0] = 0x01; // ReportId
        data[1] = 1;    // ButtonId
        data[2] = 0x01; // Press

        var report = InputReport.TryParse(data);

        Assert.That(report, Is.Not.Null);
        Assert.That(report!.ButtonId, Is.EqualTo(1));
        Assert.That(report.Event, Is.EqualTo(ButtonEventType.Press));
    }

    [Test]
    public void TryParse_WrongReportId_ReturnsNull()
    {
        var data = new byte[8];
        data[0] = 0x99;

        Assert.That(InputReport.TryParse(data), Is.Null);
    }

    [Test]
    public void TryParse_ShortData_ReturnsNull()
    {
        Assert.That(InputReport.TryParse([0x01, 0x01]), Is.Null);
    }

    [Test]
    public void OutputReport_SetLed_HasCorrectBytes()
    {
        var buf = OutputReport.SetLed(true);

        Assert.That(buf[0], Is.EqualTo(0x02));
        Assert.That(buf[1], Is.EqualTo(0x01));
        Assert.That(buf[2], Is.EqualTo(1));
    }

    [Test]
    public void TryParse_HoldEvent_ReturnsHold()
    {
        var data = new byte[8];
        data[0] = 0x01;
        data[1] = 2;
        data[2] = 0x03; // Hold

        var report = InputReport.TryParse(data);

        Assert.That(report!.Event, Is.EqualTo(ButtonEventType.Hold));
    }
}
