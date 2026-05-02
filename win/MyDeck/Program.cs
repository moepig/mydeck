using MyDeck;
using MyDeck.Command;
using MyDeck.Config;
using MyDeck.Hid;
using MyDeck.Logging;

ApplicationConfiguration.Initialize();

const string mutexName = "Global\\MyDeck-SingleInstance";
const string eventName = "Global\\MyDeck-OpenSettings";

using var guard = new SingleInstanceGuard(mutexName, eventName);

if (!guard.IsFirstInstance)
{
    if (args.Contains("--settings"))
        guard.TrySignal();
    return;
}

var configPath = Path.Combine(
    Environment.GetFolderPath(Environment.SpecialFolder.ApplicationData),
    "MyDeck", "mappings.json");

if (!File.Exists(configPath))
{
    try
    {
        ConfigLoader.Save(new AppConfig(), configPath);
    }
    catch (Exception ex)
    {
        MessageBox.Show($"設定ファイルの作成に失敗しました。\n{configPath}\n\n{ex.Message}",
                        "MyDeck", MessageBoxButtons.OK, MessageBoxIcon.Error);
        return;
    }
}

AppConfig config;
try
{
    config = ConfigLoader.Load(configPath);
}
catch (Exception ex)
{
    MessageBox.Show($"設定ファイルの読み込みに失敗しました。\n{ex.Message}", "MyDeck",
                    MessageBoxButtons.OK, MessageBoxIcon.Error);
    return;
}

var eventLog = new EventLog();

using var hid = new HidDevice();
using var app = new App(config, hid, new CommandExecutor(eventLog), eventLog, configPath, eventName);

if (args.Contains("--settings"))
    app.OpenSettings();

Application.Run();
