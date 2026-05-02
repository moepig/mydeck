#Requires -Version 5.1
<#
  MyDeck インストーラーテスト (Pester v5)

  テスト分類:
    1. 静的検査  - MSI ファイルの存在・プロパティ検証（管理者権限不要）
    2. 動的検査  - 実際のインストール・アンインストール（管理者権限必要）
                  Windows コンテナ上で実行することを想定。

  MSI パスの解決順:
    1. 環境変数 $env:MSI_PATH が設定されていればそれを使用
    2. 未設定の場合はスクリプトからの相対パスにフォールバック

  実行方法:
    # 静的検査のみ（管理者不要）
    Invoke-Pester Install.Tests.ps1 -Tag Static -Output Detailed

    # 動的検査（Windows コンテナ内、管理者権限で実行）
    $env:MSI_PATH = "C:\installer\MyDeck-Setup.msi"
    Invoke-Pester Install.Tests.ps1 -Tag Dynamic -Output Detailed
#>

BeforeAll {
    # コンテナ内では $env:MSI_PATH でパスを注入する
    $script:MsiPath = if ($env:MSI_PATH) {
        $env:MSI_PATH
    } else {
        Resolve-Path "$PSScriptRoot/../artifacts/installer/MyDeck-Setup.msi" -ErrorAction SilentlyContinue
    }
    $script:InstallDir = "$env:ProgramFiles\MyDeck"
    $script:RunKey     = "HKCU:\Software\Microsoft\Windows\CurrentVersion\Run"
    # 管理者インストールは ProgramMenuFolder が All Users に解決されるため CommonPrograms を使う
    $isAdmin = ([Security.Principal.WindowsPrincipal][Security.Principal.WindowsIdentity]::GetCurrent()).IsInRole(
        [Security.Principal.WindowsBuiltInRole]::Administrator)
    $script:StartMenuPrograms = if ($isAdmin) {
        [Environment]::GetFolderPath('CommonPrograms')
    } else {
        [Environment]::GetFolderPath('Programs')
    }

    # MSI データベースを一度だけ開いて全クエリで共有する。
    # 呼び出しごとに OpenDatabase すると COM の状態が壊れて後続クエリが null を返すため。
    # InvokeMember 経由で呼ぶことで COM 戻り値がパイプラインに漏れるのを防ぐ。
    if ($script:MsiPath) {
        $script:WiInstaller = New-Object -ComObject WindowsInstaller.Installer
        $script:WiDb = $script:WiInstaller.GetType().InvokeMember(
            'OpenDatabase', 'InvokeMethod', $null, $script:WiInstaller, @([string]$script:MsiPath, 0)
        )
    }

    function Get-MsiProperty([string]$Property) {
        if (-not $script:WiDb) { return $null }
        $view = $script:WiDb.GetType().InvokeMember(
            'OpenView', 'InvokeMethod', $null, $script:WiDb,
            @("SELECT Value FROM Property WHERE Property='$Property'")
        )
        [void]$view.GetType().InvokeMember('Execute', 'InvokeMethod', $null, $view, $null)
        $rec = $view.GetType().InvokeMember('Fetch', 'InvokeMethod', $null, $view, $null)
        $val = if ($rec) { $rec.GetType().InvokeMember('StringData', 'GetProperty', $null, $rec, @(1)) } else { $null }
        if ($rec) { [void][System.Runtime.InteropServices.Marshal]::ReleaseComObject($rec) }
        [void][System.Runtime.InteropServices.Marshal]::ReleaseComObject($view)
        return $val
    }

    function Install-Msi([string]$Path) {
        $proc = Start-Process msiexec.exe `
            -ArgumentList "/i `"$Path`" /quiet /norestart /l*v `"$env:TEMP\mydeck_install.log`"" `
            -Wait -PassThru
        return $proc.ExitCode
    }

    function Uninstall-Msi([string]$ProductCode) {
        $proc = Start-Process msiexec.exe `
            -ArgumentList "/x `"$ProductCode`" /quiet /norestart /l*v `"$env:TEMP\mydeck_uninstall.log`"" `
            -Wait -PassThru
        return $proc.ExitCode
    }
}

AfterAll {
    if ($script:WiDb)        { [void][System.Runtime.InteropServices.Marshal]::ReleaseComObject($script:WiDb) }
    if ($script:WiInstaller) { [void][System.Runtime.InteropServices.Marshal]::ReleaseComObject($script:WiInstaller) }
}

# ============================================================
#  1. 静的検査（管理者権限不要）
# ============================================================
Describe "MSI 静的検査" -Tag Static {

    It "MSI ファイルが存在する" {
        $script:MsiPath | Should -Not -BeNullOrEmpty
        $script:MsiPath | Should -Exist
    }

    It "ProductName が 'MyDeck' である" {
        $name = Get-MsiProperty "ProductName"
        $name | Should -Be "MyDeck"
    }

    It "Manufacturer が 'MyDeck' である" {
        $mfr = Get-MsiProperty "Manufacturer"
        $mfr | Should -Be "MyDeck"
    }

    It "ProductVersion が semver 形式である" {
        $ver = Get-MsiProperty "ProductVersion"
        $ver | Should -Match '^\d+\.\d+\.\d+'
    }

    It "ProductCode が GUID 形式である" {
        $code = Get-MsiProperty "ProductCode"
        $code | Should -Match '^\{[0-9A-Fa-f\-]{36}\}$'
    }

    It "UpgradeCode が GUID 形式である" {
        $code = Get-MsiProperty "UpgradeCode"
        $code | Should -Match '^\{[0-9A-Fa-f\-]{36}\}$'
    }
}

# ============================================================
#  2. 動的検査（管理者権限必要）
# ============================================================
Describe "インストール検査" -Tag Dynamic {

    BeforeAll {
        $isAdmin = ([Security.Principal.WindowsPrincipal][Security.Principal.WindowsIdentity]::GetCurrent()).IsInRole(
            [Security.Principal.WindowsBuiltInRole]::Administrator)
        if (-not $isAdmin) {
            throw "このテストブロックは管理者権限で実行してください。"
        }

        $script:ProductCode = Get-MsiProperty "ProductCode"
        $script:ExitCode    = Install-Msi $script:MsiPath
    }

    It "msiexec がエラーなく終了する（終了コード 0）" {
        $script:ExitCode | Should -Be 0
    }

    It "MyDeck.exe がインストール先に存在する" {
        "$script:InstallDir\MyDeck.exe" | Should -Exist
    }

    It "インストール先ディレクトリが存在する" {
        $script:InstallDir | Should -Exist
    }

    It "スタートメニューショートカットが存在する" {
        $shortcut = "$script:StartMenuPrograms\MyDeck\MyDeck.lnk"
        $shortcut | Should -Exist
    }

    It "自動起動レジストリエントリが存在する" {
        $val = Get-ItemPropertyValue $script:RunKey -Name "MyDeck" -ErrorAction SilentlyContinue
        $val | Should -Not -BeNullOrEmpty
        $val | Should -Match "MyDeck\.exe$"
    }
}

Describe "アンインストール検査" -Tag Dynamic {

    BeforeAll {
        $isAdmin = ([Security.Principal.WindowsPrincipal][Security.Principal.WindowsIdentity]::GetCurrent()).IsInRole(
            [Security.Principal.WindowsBuiltInRole]::Administrator)
        if (-not $isAdmin) {
            throw "このテストブロックは管理者権限で実行してください。"
        }

        $script:ProductCode  = Get-MsiProperty "ProductCode"
        $script:UninstallExit = Uninstall-Msi $script:ProductCode
    }

    It "msiexec アンインストールがエラーなく終了する（終了コード 0）" {
        $script:UninstallExit | Should -Be 0
    }

    It "MyDeck.exe が削除されている" {
        "$script:InstallDir\MyDeck.exe" | Should -Not -Exist
    }

    It "インストール先ディレクトリが削除されている" {
        $script:InstallDir | Should -Not -Exist
    }

    It "スタートメニューショートカットが削除されている" {
        $shortcut = "$script:StartMenuPrograms\MyDeck\MyDeck.lnk"
        $shortcut | Should -Not -Exist
    }

    It "自動起動レジストリエントリが削除されている" {
        $val = Get-ItemPropertyValue $script:RunKey -Name "MyDeck" -ErrorAction SilentlyContinue
        $val | Should -BeNullOrEmpty
    }
}
