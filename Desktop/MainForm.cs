using System.Diagnostics;
using System.Net;
using System.Net.Http.Headers;
using System.Security.Cryptography;
using System.Text;
using System.Text.Json;

namespace QQBot.Desktop;

public sealed class MainForm : AntdUI.Window
{
    private readonly HttpClient _http = new() { Timeout = TimeSpan.FromSeconds(8) };
    private readonly System.Windows.Forms.Timer _refreshTimer = new() { Interval = 1800 };
    private readonly string _root = AppContext.BaseDirectory.TrimEnd(Path.DirectorySeparatorChar);
    private Process? _core;
    private string _adminToken = "";
    private bool _loggedIn;

    private readonly AntdUI.Panel _surface = new() { Dock = DockStyle.Fill, Back = Color.FromArgb(245, 247, 250) };
    private readonly AntdUI.Panel _loginCard = new() { Back = Color.White, Radius = 14, Shadow = 12, Size = new Size(520, 510) };
    private readonly AntdUI.Input _appId = new() { PlaceholderText = "请输入 QQ 官方机器人 AppID", AllowClear = true, Size = new Size(410, 42) };
    private readonly AntdUI.Input _appSecret = new() { PlaceholderText = "请输入 AppSecret", UseSystemPasswordChar = true, AllowClear = true, Size = new Size(410, 42) };
    private readonly AntdUI.Select _mode = new() { Size = new Size(410, 42), DropDownArrow = true };
    private readonly AntdUI.Button _loginButton = new() { Text = "连接机器人", Size = new Size(410, 46), Radius = 8 };
    private readonly AntdUI.Label _loginHint = new() { Text = "凭据仅用于本机连接，管理接口仅监听 127.0.0.1。", ForeColor = Color.Gray, AutoSize = true };

    private readonly AntdUI.Panel _sidebar = new() { Back = Color.White, Size = new Size(210, 700) };
    private readonly AntdUI.Panel _content = new() { Back = Color.FromArgb(245, 247, 250) };
    private readonly AntdUI.Label _pageTitle = new() { Text = "运行概览", Font = new Font("Microsoft YaHei UI", 18, FontStyle.Bold), AutoSize = true };
    private readonly AntdUI.Label _connectionTag = new() { Text = "尚未连接", AutoSize = true, ForeColor = Color.Gray };
    private readonly AntdUI.Label _statusText = new() { Text = "等待机器人登录", AutoSize = true };
    private readonly AntdUI.Input _logs = new() { Multiline = true, ReadOnly = true, AutoScroll = true, WordWrap = false, Font = new Font("Consolas", 9.5f), Size = new Size(820, 440) };
    private readonly AntdUI.Button _navOverview = new() { Text = "运行概览", Size = new Size(176, 42), Ghost = true, TextAlign = ContentAlignment.MiddleLeft };
    private readonly AntdUI.Button _navMessages = new() { Text = "消息日志", Size = new Size(176, 42), Ghost = true, TextAlign = ContentAlignment.MiddleLeft };
    private readonly AntdUI.Button _navSettings = new() { Text = "连接设置", Size = new Size(176, 42), Ghost = true, TextAlign = ContentAlignment.MiddleLeft };
    private readonly AntdUI.Button _logoutButton = new() { Text = "退出连接", Size = new Size(176, 40), Ghost = true };

    public MainForm()
    {
        Text = "QQBot 官方机器人宿主";
        Size = new Size(1180, 780);
        MinimumSize = new Size(980, 680);
        StartPosition = FormStartPosition.CenterScreen;
        Mode = AntdUI.TAMode.Auto;
        BuildLogin();
        BuildDashboard();
        _refreshTimer.Tick += async (_, _) => await RefreshDataAsync();
        _loginButton.Click += async (_, _) => await LoginAsync();
        _navOverview.Click += (_, _) => ShowPage("运行概览");
        _navMessages.Click += (_, _) => ShowPage("消息日志");
        _navSettings.Click += (_, _) => ShowPage("连接设置");
        _logoutButton.Click += (_, _) => Logout();
        FormClosed += (_, _) => ShutdownCore();
        StartCore();
    }

    private void BuildLogin()
    {
        _surface.Controls.Clear();
        Controls.Add(_surface);
        var brand = new AntdUI.Label
        {
            Text = "QQBot",
            Font = new Font("Microsoft YaHei UI", 26, FontStyle.Bold),
            ForeColor = Color.FromArgb(22, 119, 255),
            AutoSize = true,
            Location = new Point(190, 48)
        };
        var subtitle = new AntdUI.Label
        {
            Text = "官方机器人桌面宿主",
            Font = new Font("Microsoft YaHei UI", 12),
            ForeColor = Color.FromArgb(90, 98, 110),
            AutoSize = true,
            Location = new Point(194, 94)
        };
        AddLoginField("AppID", _appId, 160);
        AddLoginField("AppSecret", _appSecret, 252);
        var modeLabel = MakeLabel("连接方式", 54, 350);
        _mode.Items.AddRange(new object[] { "WSS（WebSocket）", "Webhook（HTTP 回调）" });
        _mode.SelectedIndex = 0;
        _mode.Location = new Point(54, 378);
        _loginButton.Location = new Point(54, 442);
        _loginHint.Location = new Point(54, 478);
        _loginCard.Controls.AddRange(new Control[] { brand, subtitle, modeLabel, _appId, _appSecret, _mode, _loginButton, _loginHint });
        _loginCard.Location = new Point((ClientSize.Width - _loginCard.Width) / 2, (ClientSize.Height - _loginCard.Height) / 2);
        _loginCard.Anchor = AnchorStyles.None;
        _surface.Controls.Add(_loginCard);
        Resize += (_, _) =>
        {
            if (!_loggedIn)
                _loginCard.Location = new Point(Math.Max(20, (_surface.ClientSize.Width - _loginCard.Width) / 2),
                    Math.Max(20, (_surface.ClientSize.Height - _loginCard.Height) / 2));
        };
    }

    private void AddLoginField(string caption, AntdUI.Input input, int y)
    {
        var label = MakeLabel(caption, 54, y);
        input.Location = new Point(54, y + 28);
        _loginCard.Controls.Add(label);
        _loginCard.Controls.Add(input);
    }

    private static AntdUI.Label MakeLabel(string text, int x, int y) =>
        new() { Text = text, Font = new Font("Microsoft YaHei UI", 10, FontStyle.Bold), AutoSize = true, Location = new Point(x, y) };

    private void BuildDashboard()
    {
        _sidebar.Location = new Point(0, 0);
        _sidebar.Dock = DockStyle.Left;
        var brand = new AntdUI.Label { Text = "QQBot", Font = new Font("Microsoft YaHei UI", 20, FontStyle.Bold), ForeColor = Color.FromArgb(22, 119, 255), Location = new Point(24, 26), AutoSize = true };
        var caption = new AntdUI.Label { Text = "官方机器人宿主", ForeColor = Color.Gray, Location = new Point(26, 62), AutoSize = true };
        _navOverview.Location = new Point(16, 122);
        _navMessages.Location = new Point(16, 172);
        _navSettings.Location = new Point(16, 222);
        _logoutButton.Location = new Point(16, 630);
        _sidebar.Controls.AddRange(new Control[] { brand, caption, _navOverview, _navMessages, _navSettings, _logoutButton });

        _content.Dock = DockStyle.Fill;
        _pageTitle.Location = new Point(34, 28);
        _connectionTag.Location = new Point(36, 78);
        _statusText.Location = new Point(36, 116);
        var logTitle = MakeLabel("最近运行日志 / 消息记录", 36, 166);
        _logs.Location = new Point(36, 198);
        _logs.Anchor = AnchorStyles.Top | AnchorStyles.Bottom | AnchorStyles.Left | AnchorStyles.Right;
        _logs.Size = new Size(880, 470);
        _content.Controls.AddRange(new Control[] { _pageTitle, _connectionTag, _statusText, logTitle, _logs });
        _content.Visible = false;
        Controls.Add(_sidebar);
        Controls.Add(_content);
        _sidebar.Visible = false;
    }

    private void StartCore()
    {
        var corePath = Path.Combine(_root, "runtime", "qqbot-core.exe");
        if (!File.Exists(corePath))
        {
            AntdUI.Message.error(this, "未找到 runtime\\qqbot-core.exe，请检查安装包目录。");
            return;
        }

        _adminToken = Convert.ToHexString(RandomNumberGenerator.GetBytes(32)).ToLowerInvariant();
        var psi = new ProcessStartInfo(corePath)
        {
            WorkingDirectory = _root,
            UseShellExecute = false,
            CreateNoWindow = true
        };
        psi.Environment["QQBOT_ADMIN_TOKEN"] = _adminToken;
        psi.Environment["QQBOT_BIND"] = "127.0.0.1";
        psi.Environment["QQBOT_PORT"] = "8080";
        psi.Environment["CURL_CA_BUNDLE"] = Path.Combine(_root, "certs", "ca-bundle.crt");
        psi.Environment["PATH"] = Path.Combine(_root, "runtime") + Path.PathSeparator + (Environment.GetEnvironmentVariable("PATH") ?? "");
        try
        {
            _core = Process.Start(psi);
            _http.DefaultRequestHeaders.Authorization = new AuthenticationHeaderValue("Bearer", _adminToken);
        }
        catch (Exception ex)
        {
            AntdUI.Message.error(this, ex.Message);
        }
    }

    private async Task LoginAsync()
    {
        if (string.IsNullOrWhiteSpace(_appId.Text) || string.IsNullOrWhiteSpace(_appSecret.Text))
        {
            AntdUI.Message.warn(this, "请填写 AppID 和 AppSecret。");
            return;
        }
        _loginButton.Loading = true;
        try
        {
            var mode = _mode.SelectedIndex == 0 ? "websocket" : "webhook";
            var payload = JsonSerializer.Serialize(new { app_id = _appId.Text.Trim(), app_secret = _appSecret.Text, mode });
            using var response = await _http.PostAsync("http://127.0.0.1:8080/api/v1/auth/login",
                new StringContent(payload, Encoding.UTF8, "application/json"));
            var body = await response.Content.ReadAsStringAsync();
            if (!response.IsSuccessStatusCode)
                throw new InvalidOperationException(ExtractError(body));
            _appSecret.Text = "";
            _loggedIn = true;
            _surface.Visible = false;
            _sidebar.Visible = true;
            _content.Visible = true;
            _refreshTimer.Start();
            await RefreshDataAsync();
            AntdUI.Message.success(this, "机器人登录请求已提交。");
        }
        catch (Exception ex)
        {
            AntdUI.Message.error(this, ex.Message);
        }
        finally
        {
            _loginButton.Loading = false;
        }
    }

    private static string ExtractError(string body)
    {
        try
        {
            using var doc = JsonDocument.Parse(body);
            if (doc.RootElement.TryGetProperty("message", out var msg)) return msg.GetString() ?? body;
            if (doc.RootElement.TryGetProperty("error", out var err)) return err.GetString() ?? body;
        }
        catch { }
        return body;
    }

    private async Task RefreshDataAsync()
    {
        if (!_loggedIn) return;
        try
        {
            var status = await _http.GetStringAsync("http://127.0.0.1:8080/api/v1/status");
            var logs = await _http.GetStringAsync("http://127.0.0.1:8080/api/v1/logs");
            _statusText.Text = "运行状态：" + FormatJson(status);
            _logs.Text = FormatJson(logs);
            _connectionTag.Text = status.Contains("\"connected\"") ? "WSS 已连接" :
                status.Contains("webhook_active") ? "Webhook 已启用" : "正在等待连接状态";
            _connectionTag.ForeColor = status.Contains("\"connected\"") || status.Contains("webhook_active")
                ? Color.FromArgb(82, 196, 26) : Color.FromArgb(250, 173, 20);
        }
        catch (Exception ex)
        {
            _connectionTag.Text = "本机宿主连接异常";
            _connectionTag.ForeColor = Color.FromArgb(255, 77, 79);
            _statusText.Text = ex.Message;
        }
    }

    private static string FormatJson(string json)
    {
        try { return JsonSerializer.Serialize(JsonDocument.Parse(json).RootElement, new JsonSerializerOptions { WriteIndented = true }); }
        catch { return json; }
    }

    private void ShowPage(string title)
    {
        _pageTitle.Text = title;
        _logs.Visible = title != "连接设置";
        _statusText.Visible = title != "消息日志";
        if (title == "连接设置")
            _statusText.Text = "当前接入模式和连接状态可在运行概览查看。Webhook 需要公网 HTTPS 回调地址，并在 QQ 开放平台配置 /webhook/qq。";
        else if (title == "消息日志")
            _statusText.Text = "显示宿主收到的最近日志；当前首批覆盖群 @ 消息与 C2C 单聊消息。";
        else
            _statusText.Text = "本机宿主状态与最近事件日志每 1.8 秒自动刷新。";
        _ = RefreshDataAsync();
    }

    private void Logout()
    {
        _refreshTimer.Stop();
        _loggedIn = false;
        _content.Visible = false;
        _sidebar.Visible = false;
        _surface.Visible = true;
        _appSecret.Text = "";
    }

    private void ShutdownCore()
    {
        _refreshTimer.Stop();
        try
        {
            if (_core is { HasExited: false })
            {
                _core.Kill(true);
                _core.WaitForExit(2500);
            }
        }
        catch { }
        _http.Dispose();
    }
}
