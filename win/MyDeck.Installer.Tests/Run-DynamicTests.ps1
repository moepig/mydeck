#Requires -Version 5.1
<#
.SYNOPSIS
  Windows Docker コンテナ内で MyDeck インストーラー Dynamic テストを実行する。

.DESCRIPTION
  ローカル環境を汚染せずに Dynamic テスト（実インストール・アンインストール）を実行するため、
  Windows コンテナを使用する。テスト終了後はコンテナを自動削除する。

  前提条件:
    - Docker Desktop がインストール済みで起動していること
    - Windows コンテナモードに切り替え済みであること
      ( Docker Desktop タスクトレイアイコンを右クリック → "Switch to Windows containers" )

.PARAMETER MsiPath
  テスト対象の MSI ファイルパス。
  省略時は artifacts\installer\MyDeck-Setup.msi を自動検索する。

.EXAMPLE
  # 管理者権限は不要（コンテナ内で処理するため）
  .\Run-DynamicTests.ps1

.EXAMPLE
  .\Run-DynamicTests.ps1 -MsiPath "C:\build\MyDeck-Setup.msi"
#>
[CmdletBinding()]
param(
    [string] $MsiPath
)

Set-StrictMode -Version Latest
$ErrorActionPreference = "Stop"

# ---------------------------------------------------------------
# 1. Docker の確認
# ---------------------------------------------------------------
if (-not (Get-Command docker -ErrorAction SilentlyContinue)) {
    Write-Error "docker コマンドが見つかりません。Docker Desktop をインストールしてください。"
    exit 1
}

docker info 2>&1 | Out-Null
if ($LASTEXITCODE -ne 0) {
    Write-Error "Docker が起動していません。Docker Desktop を起動してから再実行してください。"
    exit 1
}

$osType = docker info --format '{{.OSType}}'
if ($osType -ne 'windows') {
    Write-Error @"
Docker が Windows コンテナモードではありません（現在: $osType）。
Docker Desktop のタスクトレイアイコンを右クリック →
  "Switch to Windows containers" に切り替えてから再実行してください。
"@
    exit 1
}

# ---------------------------------------------------------------
# 2. MSI パスの解決
# ---------------------------------------------------------------
if (-not $MsiPath) {
    $candidates = @(
        (Join-Path $PSScriptRoot "..\artifacts\installer\MyDeck-Setup.msi"),
        (Join-Path $PSScriptRoot "..\..\artifacts\installer\MyDeck-Setup.msi")
    )
    foreach ($c in $candidates) {
        $resolved = Resolve-Path $c -ErrorAction SilentlyContinue
        if ($resolved) { $MsiPath = $resolved.Path; break }
    }
}

if (-not $MsiPath -or -not (Test-Path $MsiPath)) {
    Write-Error @"
MSI ファイルが見つかりません。先にインストーラーをビルドするか、-MsiPath で場所を指定してください。

ビルド手順:
  cd win
  dotnet publish MyDeck/MyDeck.csproj -c Release -r win-x64 --self-contained -o artifacts/publish
  dotnet build MyDeck.Installer/MyDeck.Installer.wixproj -c Release
  copy MyDeck.Installer\bin\Release\MyDeck-Setup.msi artifacts\installer\
"@
    exit 1
}

$MsiPath  = (Resolve-Path $MsiPath).Path
$MsiDir   = Split-Path $MsiPath -Parent
$MsiFile  = Split-Path $MsiPath -Leaf
$TestsDir = $PSScriptRoot

Write-Host "MSI:      $MsiPath"  -ForegroundColor Cyan
Write-Host "テスト:   $TestsDir" -ForegroundColor Cyan

# ---------------------------------------------------------------
# 3. コンテナイメージの確認・取得
# ---------------------------------------------------------------
$image = "mcr.microsoft.com/powershell:windowsservercore-ltsc2022"
Write-Host ""
Write-Host "イメージを確認しています: $image" -ForegroundColor Yellow

docker pull $image
if ($LASTEXITCODE -ne 0) {
    Write-Error "イメージの取得に失敗しました: $image"
    exit 1
}

# ---------------------------------------------------------------
# 4. コンテナ内スクリプトを base64 エンコード
#    docker run 引数へ直接渡す際のエスケープ問題を回避する
# ---------------------------------------------------------------
$innerScript = @'
Set-ExecutionPolicy Bypass -Scope Process -Force
Install-Module Pester -Force -SkipPublisherCheck -MinimumVersion 5.0.0 -Scope CurrentUser
Import-Module Pester -MinimumVersion 5.0.0

$config = New-PesterConfiguration
$config.Run.Path         = 'C:\tests\Install.Tests.ps1'
$config.Filter.Tag       = 'Dynamic'
$config.Output.Verbosity = 'Detailed'
$config.Run.PassThru     = $true

$result = Invoke-Pester -Configuration $config
exit ($result.FailedCount -gt 0 ? 1 : 0)
'@

$encodedCommand = [Convert]::ToBase64String([Text.Encoding]::Unicode.GetBytes($innerScript))

# ---------------------------------------------------------------
# 5. コンテナ起動・テスト実行
#    --rm  : 終了後にコンテナを自動削除（ローカル環境へ影響なし）
#    -v    : MSI ディレクトリとテストスクリプトをコンテナにマウント
#    -e    : MSI パスを環境変数で注入
# ---------------------------------------------------------------
Write-Host ""
Write-Host "=== Windows コンテナでテストを実行します ===" -ForegroundColor Green
Write-Host ""

docker run --rm `
    --isolation process `
    -v "${MsiDir}:C:\installer" `
    -v "${TestsDir}:C:\tests" `
    -e "MSI_PATH=C:\installer\${MsiFile}" `
    $image `
    pwsh -NonInteractive -EncodedCommand $encodedCommand

$exitCode = $LASTEXITCODE

# ---------------------------------------------------------------
# 6. 結果表示
# ---------------------------------------------------------------
Write-Host ""
if ($exitCode -ne 0) {
    Write-Host "テスト失敗（終了コード: $exitCode）" -ForegroundColor Red
} else {
    Write-Host "全テスト通過" -ForegroundColor Green
}
exit $exitCode
