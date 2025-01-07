using System;
using System.Runtime.InteropServices;

namespace GNet;

/*
typedef enum G_APPLICATION_FLAGS
{
  G_APPLICATION_FLAGS_NONE GIO_DEPRECATED_ENUMERATOR_IN_2_74_FOR(G_APPLICATION_DEFAULT_FLAGS),
  G_APPLICATION_DEFAULT_FLAGS GIO_AVAILABLE_ENUMERATOR_IN_2_74 = 0,
  G_APPLICATION_IS_SERVICE  =          (1 << 0),
  G_APPLICATION_IS_LAUNCHER =          (1 << 1),

  G_APPLICATION_HANDLES_OPEN =         (1 << 2),
  G_APPLICATION_HANDLES_COMMAND_LINE = (1 << 3),
  G_APPLICATION_SEND_ENVIRONMENT    =  (1 << 4),

  G_APPLICATION_NON_UNIQUE =           (1 << 5),

  G_APPLICATION_CAN_OVERRIDE_APP_ID =  (1 << 6),
  G_APPLICATION_ALLOW_REPLACEMENT   =  (1 << 7),
  G_APPLICATION_REPLACE             =  (1 << 8)
} GApplicationFlags;
*/

public static class GAppTest
{
    public delegate void GCallback(IntPtr obj, IntPtr data);

#pragma warning disable CA2101 // Specify marshaling for P/Invoke string arguments
    [DllImport("gio-2.0", EntryPoint = "g_application_new")]
    private static extern IntPtr GApplication(string strId, int nFlags);
    [DllImport("gio-2.0", EntryPoint = "g_application_run")]
    private static extern int GApplicationMain(IntPtr obj, int argc, IntPtr argv);
    [DllImport("gobject-2.0", EntryPoint = "g_object_unref")]
    private static extern void GRelease(IntPtr obj);
    [DllImport("gobject-2.0", EntryPoint = "g_object_ref")]
    private static extern void GRetain(IntPtr obj);
    [DllImport("gobject-2.0", EntryPoint = "g_signal_connect_data")]
    private static extern void GSignalConnectData(IntPtr obj, string detail, GCallback closure, IntPtr data, IntPtr notify, int flags);

    [DllImport("gtk-4", EntryPoint = "gtk_application_new")]
    private static extern IntPtr GtkApplication(string strId, int flags);
    [DllImport("gtk-4", EntryPoint = "gtk_application_window_new")]
    private static extern IntPtr GtkApplicationWindow(IntPtr app);
    [DllImport("gtk-4", EntryPoint = "gtk_window_set_default_size")]
    private static extern void GtkWindowSetDefaultSize(IntPtr window, int width, int height);
    [DllImport("gtk-4", EntryPoint = "gtk_window_set_title")]
    private static extern void GtkWindowSetTitle(IntPtr window, string title);
    [DllImport("gtk-4", EntryPoint = "gtk_window_present")]
    private static extern void GtkWindowShow(IntPtr window);
#pragma warning restore CA2101 // Specify marshaling for P/Invoke string arguments

    private static void GSignalConnect(IntPtr obj, string signalName, GCallback cb, IntPtr data)
    {
        GSignalConnectData(obj, signalName, cb, data, IntPtr.Zero, 0);
    }

    public static void OnActivate(IntPtr app, IntPtr data)
    {
        Console.WriteLine($"\x1b[1;34mGAppTest.OnActivate\x1b[0m: app = {app}; data = {data}");
        IntPtr window = GtkApplicationWindow(app);
        GtkWindowSetDefaultSize(window, 320, 350);
        GtkWindowSetTitle(window, "Gtk4 .NET Host - Test");
        GtkWindowShow(window);
    }

    [UnmanagedCallersOnly]
    public static int Run(int argc, IntPtr argv)
    {
        Console.WriteLine($"Running with {argc} arguments...\nargv is: {argv}");

        IntPtr obj = GtkApplication("net.adi.glib-test1", 0);
        if (obj == IntPtr.Zero)
        {
            // Complain
            Console.WriteLine($"\x1b[1;31mError\x1b[0m Could not create GApplication");
            return 1;
        }
        
        Console.WriteLine($"\x1b[1;34mtest1\x1b[0m Created GApplication object at {obj}");

        GSignalConnect(obj, "activate", OnActivate, IntPtr.Zero);

        int res = GApplicationMain(obj, 0, argv);
        GRelease(obj);
        Console.WriteLine($"\x1b[1;34mtest1\x1b[0m GApplicationMain returns status: {res}");

        return res;
    }
}

