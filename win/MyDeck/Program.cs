using MyDeck;
using MyDeck.Command;
using MyDeck.Config;
using MyDeck.Hid;

ApplicationConfiguration.Initialize();

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
using var app = new App(config, hid, new CommandExecutor(), configPath);

Application.Run();
