using System;
using System.Windows.Forms;

namespace QQBot.Desktop;

internal static class Program
{
    [STAThread]
    private static void Main()
    {
        Application.SetHighDpiMode(HighDpiMode.PerMonitorV2);
        Application.EnableVisualStyles();
        Application.SetCompatibleTextRenderingDefault(false);

        AntdUI.Localization.DefaultLanguage = "zh-CN";
        AntdUI.Config.TextRenderingHint = System.Drawing.Text.TextRenderingHint.AntiAliasGridFit;
        AntdUI.Config.TextRenderingHighQuality = true;
        AntdUI.Config.Theme().Dark("#141414", "#ffffff").Light("#ffffff", "#141414").FormBorderColor();

        Application.ThreadException += (_, e) =>
            AntdUI.Message.error(e.Exception.Message, "界面发生异常");
        AppDomain.CurrentDomain.UnhandledException += (_, e) =>
            MessageBox.Show(e.ExceptionObject?.ToString() ?? "未知异常", "QQBot", MessageBoxButtons.OK, MessageBoxIcon.Error);

        Application.Run(new MainForm());
    }
}
