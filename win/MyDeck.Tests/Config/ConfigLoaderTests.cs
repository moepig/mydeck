using MyDeck.Config;
using NUnit.Framework;

namespace MyDeck.Tests.Config;

[TestFixture]
public class ConfigLoaderTests
{
    private string _tmpFile = string.Empty;

    [SetUp]
    public void SetUp()
    {
        _tmpFile = Path.GetTempFileName();
    }

    [TearDown]
    public void TearDown()
    {
        File.Delete(_tmpFile);
    }

    private void WriteJson(string json) => File.WriteAllText(_tmpFile, json);

    [Test]
    public void Load_ValidConfig_ParsesCorrectly()
    {
        WriteJson("""
        {
          "version": 1,
          "device": { "vendorId": 6991, "productId": 37382 },
          "buttons": [
            {
              "buttonId": 1,
              "label": "Test",
              "actions": [
                { "event": "press", "type": "process",
                  "executable": "notepad.exe", "arguments": "" }
              ]
            }
          ]
        }
        """);

        var config = ConfigLoader.Load(_tmpFile);

        Assert.That(config.Version, Is.EqualTo(1));
        Assert.That(config.Buttons, Has.Count.EqualTo(1));
        Assert.That(config.Buttons[0].ButtonId, Is.EqualTo(1));
        Assert.That(config.Buttons[0].Actions[0].Executable, Is.EqualTo("notepad.exe"));
    }

    [Test]
    public void Load_DuplicateButtonId_Throws()
    {
        WriteJson("""
        {
          "version": 1,
          "buttons": [
            { "buttonId": 1, "label": "A", "actions": [] },
            { "buttonId": 1, "label": "B", "actions": [] }
          ]
        }
        """);

        Assert.Throws<InvalidDataException>(() => ConfigLoader.Load(_tmpFile));
    }

    [Test]
    public void Load_InvalidEventName_Throws()
    {
        WriteJson("""
        {
          "version": 1,
          "buttons": [
            {
              "buttonId": 1, "label": "A",
              "actions": [
                { "event": "double_click", "type": "process",
                  "executable": "notepad.exe", "arguments": "" }
              ]
            }
          ]
        }
        """);

        Assert.Throws<InvalidDataException>(() => ConfigLoader.Load(_tmpFile));
    }
}
