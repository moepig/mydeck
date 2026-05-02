using MyDeck;
using MyDeck.Command;
using MyDeck.Config;
using MyDeck.Hid;

ApplicationConfiguration.Initialize();

const string mutexName = "Global\\MyDeck-SingleInstance";
const string eventName = "Global\\MyDeck-OpenSettings";

using var guard = new SingleInstanceGuard(mutexName, eventName);

if (!guard.IsFirstInstance)
{
    // 既に起動中 — --settings なら設定画面を開くようシグナルを送って終了
    if (args.Contains("--settings"))
        guard.TrySignal();
    return;
}

var configPath = Path.Combine(
    Environment.GetFolderPath(Environment.SpecialFolder.ApplicationData),
    "MyDeck", "mappings.json");

// 設定ファイルが存在しない場合は空の設定で新規作成する
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

using var hid = new HidDevice();
using var app = new App(config, hid, new CommandExecutor(), configPath, eventName);

if (args.Contains("--settings"))
    app.OpenSettings();

Application.Run();
