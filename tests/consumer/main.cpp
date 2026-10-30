#include <ElaApplication.h>
#include <ElaIcon.h>
#include <ElaTabBar.h>
#include <ElaTheme.h>

#include <QApplication>
#include <QFontDatabase>
#include <QImage>
#include <QTimer>

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    eApp->init();
    if (!QFontDatabase::families().contains("ElaAwesome") || QImage(":/include/Image/Cirno.jpg").isNull() ||
        ElaIcon::getInstance()->getElaIcon(ElaIconType::House).pixmap(24, 24).isNull())
    {
        qCritical("安装后的字体、图像或图标资源不可用");
        return 1;
    }
    ElaTabBar tabs;
    tabs.addTab("首页");
    tabs.addTab("设置");
    tabs.resize(400, 50);
    tabs.show();
    QTimer::singleShot(0, &app, [&]() {
        eTheme->setThemeMode(ElaThemeType::Dark);
        if (tabs.grab().isNull())
        {
            app.exit(2);
            return;
        }
        app.quit();
    });
    return app.exec();
}
