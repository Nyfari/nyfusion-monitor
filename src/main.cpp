#include <QByteArray>
#include <QCoreApplication>
#include <QDir>
#include <QDebug>
#include <QtWidgets/QApplication>

#include "qt/MainWindow.hpp"

/**
 * Garante que o app rode via XWayland quando o sistema usa Wayland.
 *
 * Wayland, por segurança, impede qualquer janela de cliente de se forçar acima
 * de outra janela fullscreen de outro cliente.  Para um overlay de jogo isso
 * é fatal — a única solução sem implementar wlr-layer-shell do zero é usar o
 * backend X11 (XWayland), que:
 *   • Suporta override_redirect (BypassWindowManagerHint) nativo.
 *   • Cobre 99 % dos jogos no Linux (Steam/Proton/Wine rodam em XWayland).
 *   • Funciona em KDE, GNOME, Sway, Hyprland — desde que XWayland esteja ativo.
 *
 * A variável precisa ser definida ANTES de qualquer QApplication ou
 * QGuiApplication ser instanciado.
 */
static void enforceXWaylandIfNeeded() {
    // Se o usuário já forçou explicitamente uma plataforma, respeitamos.
    if (!qgetenv("QT_QPA_PLATFORM").isEmpty()) {
        return;
    }

    const bool isWayland =
        qgetenv("XDG_SESSION_TYPE") == "wayland"
        || !qgetenv("WAYLAND_DISPLAY").isEmpty();

    if (!isWayland) {
        return; // Já é X11 nativo — nada a fazer.
    }

    // Verifica se um servidor X está disponível (XWayland).
    if (qgetenv("DISPLAY").isEmpty()) {
        // XWayland não está rodando: sem overlay, mas o app ainda funciona.
        return;
    }

    // Força o backend XCB (X11/XWayland) para que o overlay possa usar
    // override_redirect e ficar acima de jogos fullscreen.
    qputenv("QT_QPA_PLATFORM", "xcb");
}

int main(int argc, char* argv[]) {
    enforceXWaylandIfNeeded();

    QApplication app(argc, argv);

    // ── Imprime instrução do hook LD_PRELOAD ──────────────────────────────────
    const QString hookPath = QDir(QCoreApplication::applicationDirPath())
                                .filePath(QStringLiteral("libny_fps_hook.so"));
    qInfo().noquote()
        << "\n╔══════════════════════════════════════════════════════════════╗"
        << "\n║  NyFusion Monitor — para FPS real adicione ao Steam:         ║"
        << "\n║  Opções de lançamento do jogo:                                ║"
        << "\n║  LD_PRELOAD=" + hookPath + " %command%"
        << "\n╚══════════════════════════════════════════════════════════════╝\n";

    ny::ui::qt::MainWindow window;
    window.show();

    return app.exec();
}
