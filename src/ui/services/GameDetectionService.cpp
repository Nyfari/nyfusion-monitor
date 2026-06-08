/**
 * @file GameDetectionService.cpp
 * @author Marcos Henrique
 * @date 16/05/2026
 */
#include "GameDetectionService.hpp"

#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QProcess>
#include <QProcessEnvironment>
#include <QRegularExpression>
#include <QSet>
#include <QStringList>
#include <QStandardPaths>
#include <QtGlobal>

#if defined(_WIN32)
#include <windows.h>
#include <psapi.h>
#include <tlhelp32.h>
#endif

namespace ny::ui::services {
namespace {
struct KnownGamesCatalog final {
    QStringList installRoots;
    QStringList gameNames;
    int indexedGames{0};
};

struct ActiveWindowInfo final {
    qint64 processId{0};
    QString title;
    QString executablePath;
    bool fullscreen{false};
};

QString readTrimmedFile(const QString& filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return {};
    }

    return QString::fromUtf8(file.readAll()).trimmed();
}

QString readCmdline(const QString& filePath) {
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly)) {
        return {};
    }

    QByteArray data = file.readAll();
    data.replace('\0', ' ');
    return QString::fromUtf8(data).trimmed();
}

QString runProcessOutput(const QString& program, const QStringList& arguments, const int timeoutMs = 1000) {
    QProcess process;
    process.start(program, arguments);
    if (!process.waitForStarted(250) || !process.waitForFinished(timeoutMs)) {
        return {};
    }

    return QString::fromUtf8(process.readAllStandardOutput()).trimmed();
}

qint64 readMemoryRssKb(const QString& statusFilePath) {
    QFile file(statusFilePath);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return 0;
    }

    while (!file.atEnd()) {
        const QString line = QString::fromUtf8(file.readLine()).trimmed();
        if (!line.startsWith(QStringLiteral("VmRSS:"))) {
            continue;
        }

        const QStringList parts = line.split(QRegularExpression(QStringLiteral("\\s+")), Qt::SkipEmptyParts);
        if (parts.size() >= 2) {
            bool ok = false;
            const qint64 value = parts.at(1).toLongLong(&ok);
            if (ok) {
                return value;
            }
        }
        break;
    }

    return 0;
}

QString displayNameForCandidate(const QString& processName, const QString& executablePath, const QString& cmdline) {
    QFileInfo executableInfo(executablePath);
    QString candidate = executableInfo.completeBaseName();
    if (candidate.isEmpty()) {
        candidate = executableInfo.fileName();
    }
    if (candidate.isEmpty()) {
        candidate = processName;
    }
    if (candidate.isEmpty() && !cmdline.isEmpty()) {
        candidate = cmdline.section(' ', 0, 0);
    }

    candidate.replace(QStringLiteral(".x86_64"), QString());
    candidate.replace(QStringLiteral(".exe"), QString(), Qt::CaseInsensitive);
    candidate.replace(QStringLiteral("_"), QStringLiteral(" "));
    return candidate.trimmed();
}

void addGameDirectoriesFromRoot(QSet<QString>& roots, QSet<QString>& names, const QString& rootPath) {
    const QDir root(rootPath);
    if (!root.exists()) {
        return;
    }

    const QFileInfoList entries = root.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);
    for (const QFileInfo& entry : entries) {
        roots.insert(QDir::cleanPath(entry.absoluteFilePath()).toLower());
        names.insert(entry.fileName().toLower());
    }
}

QStringList steamCommonRoots() {
    QStringList roots;
    const QString home = QDir::homePath();
    roots << home + QStringLiteral("/.steam/steam/steamapps/common")
          << home + QStringLiteral("/.local/share/Steam/steamapps/common")
          << home + QStringLiteral("/.var/app/com.valvesoftware.Steam/.local/share/Steam/steamapps/common")
          << home + QStringLiteral("/Library/Application Support/Steam/steamapps/common");

    const auto env = QProcessEnvironment::systemEnvironment();
    if (env.contains(QStringLiteral("ProgramFiles(x86)"))) {
        roots << env.value(QStringLiteral("ProgramFiles(x86)")) + QStringLiteral("/Steam/steamapps/common");
    }
    if (env.contains(QStringLiteral("ProgramFiles"))) {
        roots << env.value(QStringLiteral("ProgramFiles")) + QStringLiteral("/Steam/steamapps/common");
    }

    return roots;
}

QStringList steamLibraryFoldersFiles() {
    QStringList files;
    const QString home = QDir::homePath();
    files << home + QStringLiteral("/.steam/steam/steamapps/libraryfolders.vdf")
          << home + QStringLiteral("/.local/share/Steam/steamapps/libraryfolders.vdf")
          << home + QStringLiteral("/.var/app/com.valvesoftware.Steam/.local/share/Steam/steamapps/libraryfolders.vdf")
          << home + QStringLiteral("/Library/Application Support/Steam/steamapps/libraryfolders.vdf");

    const auto env = QProcessEnvironment::systemEnvironment();
    if (env.contains(QStringLiteral("ProgramFiles(x86)"))) {
        files << env.value(QStringLiteral("ProgramFiles(x86)")) + QStringLiteral("/Steam/steamapps/libraryfolders.vdf");
    }
    if (env.contains(QStringLiteral("ProgramFiles"))) {
        files << env.value(QStringLiteral("ProgramFiles")) + QStringLiteral("/Steam/steamapps/libraryfolders.vdf");
    }

    return files;
}

KnownGamesCatalog buildKnownGamesCatalog() {
    QSet<QString> roots;
    QSet<QString> names;
    const QString home = QDir::homePath();

    for (const QString& rootPath : steamCommonRoots()) {
        addGameDirectoriesFromRoot(roots, names, rootPath);
    }

    static const QRegularExpression libraryPathRe(QStringLiteral(R"REGEX("path"\s+"([^"]+)")REGEX"));
    for (const QString& libraryFilePath : steamLibraryFoldersFiles()) {
        const QString contents = readTrimmedFile(libraryFilePath);
        auto it = libraryPathRe.globalMatch(contents);
        while (it.hasNext()) {
            QString libraryPath = it.next().captured(1);
            libraryPath.replace(QStringLiteral(R"(\\)"), QStringLiteral("/"));
            addGameDirectoriesFromRoot(roots, names, QDir::cleanPath(libraryPath + QStringLiteral("/steamapps/common")));
        }
    }

    const QStringList additionalRoots = {
        home + QStringLiteral("/Games"),
        home + QStringLiteral("/Applications"),
        home + QStringLiteral("/Heroic"),
        home + QStringLiteral("/Lutris"),
        home + QStringLiteral("/Bottles"),
    };
    for (const QString& rootPath : additionalRoots) {
        addGameDirectoriesFromRoot(roots, names, rootPath);
    }

    const QStringList appDirs = QStandardPaths::standardLocations(QStandardPaths::ApplicationsLocation);
    for (const QString& appDir : appDirs) {
        addGameDirectoriesFromRoot(roots, names, appDir);
    }

    KnownGamesCatalog catalog;
    catalog.installRoots = roots.values();
    catalog.gameNames = names.values();
    catalog.indexedGames = catalog.gameNames.size();
    return catalog;
}

const KnownGamesCatalog& knownGamesCatalog() {
    static const KnownGamesCatalog catalog = buildKnownGamesCatalog();
    return catalog;
}

bool matchesInstalledGame(const QString& processName, const QString& executablePath, const KnownGamesCatalog& catalog) {
    const QString normalizedProcessName = QFileInfo(processName).completeBaseName().toLower();
    const QString normalizedExecutablePath = QDir::cleanPath(executablePath).toLower();

    for (const QString& root : catalog.installRoots) {
        if (!root.isEmpty() && normalizedExecutablePath.contains(root)) {
            return true;
        }
    }
    for (const QString& gameName : catalog.gameNames) {
        if (gameName.isEmpty()) {
            continue;
        }
        if (normalizedProcessName == gameName || normalizedExecutablePath.contains(QStringLiteral("/") + gameName + QStringLiteral("/"))) {
            return true;
        }
    }

    return false;
}

std::optional<ActiveWindowInfo> detectActiveWindowInfo() {
#if defined(__linux__)
    const QString rootInfo = runProcessOutput(QStringLiteral("xprop"), { QStringLiteral("-root"), QStringLiteral("_NET_ACTIVE_WINDOW") });
    static const QRegularExpression idRe(QStringLiteral(R"((0x[0-9a-fA-F]+))"));
    const auto idMatch = idRe.match(rootInfo);
    if (!idMatch.hasMatch()) {
        return std::nullopt;
    }

    const QString windowId = idMatch.captured(1);
    const QString details = runProcessOutput(
        QStringLiteral("xprop"),
        { QStringLiteral("-id"), windowId, QStringLiteral("_NET_WM_PID"), QStringLiteral("_NET_WM_STATE"), QStringLiteral("_NET_WM_NAME") }
    );

    static const QRegularExpression pidRe(QStringLiteral(R"(_NET_WM_PID\(CARDINAL\) = (\d+))"));
    const auto pidMatch = pidRe.match(details);
    if (!pidMatch.hasMatch()) {
        return std::nullopt;
    }

    ActiveWindowInfo info;
    info.processId = pidMatch.captured(1).toLongLong();
    info.fullscreen = details.contains(QStringLiteral("_NET_WM_STATE_FULLSCREEN"));

    static const QRegularExpression titleRe(QStringLiteral(R"REGEX(_NET_WM_NAME\(UTF8_STRING\) = "([^"]*)")REGEX"));
    const auto titleMatch = titleRe.match(details);
    if (titleMatch.hasMatch()) {
        info.title = titleMatch.captured(1).trimmed();
    }
    info.executablePath = QFileInfo(QStringLiteral("/proc/%1/exe").arg(info.processId)).symLinkTarget();
    return info.processId > 0 ? std::optional<ActiveWindowInfo>(info) : std::nullopt;
#elif defined(__APPLE__)
    const QString output = runProcessOutput(
        QStringLiteral("/usr/bin/osascript"),
        { QStringLiteral("-e"), QStringLiteral("tell application \"System Events\" to get unix id of first application process whose frontmost is true") }
    );
    bool ok = false;
    const qint64 pid = output.toLongLong(&ok);
    if (!ok || pid <= 0) {
        return std::nullopt;
    }

    ActiveWindowInfo info;
    info.processId = pid;
    info.title = runProcessOutput(
        QStringLiteral("/usr/bin/osascript"),
        { QStringLiteral("-e"), QStringLiteral("tell application \"System Events\" to get name of first application process whose frontmost is true") }
    );
    info.executablePath = runProcessOutput(QStringLiteral("/bin/ps"), { QStringLiteral("-p"), QString::number(pid), QStringLiteral("-o"), QStringLiteral("comm=") });
    return info;
#elif defined(_WIN32)
    const HWND foregroundWindow = GetForegroundWindow();
    if (!foregroundWindow) {
        return std::nullopt;
    }

    DWORD pid = 0;
    GetWindowThreadProcessId(foregroundWindow, &pid);
    if (pid == 0) {
        return std::nullopt;
    }

    ActiveWindowInfo info;
    info.processId = static_cast<qint64>(pid);

    RECT windowRect{};
    if (GetWindowRect(foregroundWindow, &windowRect)) {
        const HMONITOR monitor = MonitorFromWindow(foregroundWindow, MONITOR_DEFAULTTONEAREST);
        MONITORINFO monitorInfo{};
        monitorInfo.cbSize = sizeof(MONITORINFO);
        if (GetMonitorInfoW(monitor, &monitorInfo)) {
            info.fullscreen = windowRect.left <= monitorInfo.rcMonitor.left
                && windowRect.top <= monitorInfo.rcMonitor.top
                && windowRect.right >= monitorInfo.rcMonitor.right
                && windowRect.bottom >= monitorInfo.rcMonitor.bottom;
        }
    }

    wchar_t titleBuffer[512]{};
    const int titleLength = GetWindowTextW(foregroundWindow, titleBuffer, 512);
    if (titleLength > 0) {
        info.title = QString::fromWCharArray(titleBuffer, titleLength);
    }

    HANDLE processHandle = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);
    if (processHandle) {
        wchar_t pathBuffer[MAX_PATH]{};
        DWORD size = MAX_PATH;
        if (QueryFullProcessImageNameW(processHandle, 0, pathBuffer, &size)) {
            info.executablePath = QString::fromWCharArray(pathBuffer, static_cast<int>(size));
        }
        CloseHandle(processHandle);
    }
    return info;
#else
    return std::nullopt;
#endif
}

bool containsAny(const QString& haystack, const QStringList& needles) {
    for (const QString& needle : needles) {
        if (haystack.contains(needle, Qt::CaseInsensitive)) {
            return true;
        }
    }
    return false;
}

int heuristicScore(const QString& processName, const QString& executablePath, const QString& cmdline, const qint64 rssKb) {
    static const QStringList blacklist = {
        QStringLiteral("steam"),
        QStringLiteral("steamwebhelper"),
        QStringLiteral("heroic"),
        QStringLiteral("lutris"),
        QStringLiteral("discord"),
        QStringLiteral("chrome"),
        QStringLiteral("chromium"),
        QStringLiteral("firefox"),
        QStringLiteral("code"),
        QStringLiteral("clion"),
        QStringLiteral("jetbrains"),
        QStringLiteral("gnome-shell"),
        QStringLiteral("plasmashell"),
        QStringLiteral("explorer.exe"),
        QStringLiteral("finder"),
        QStringLiteral("windowserver"),
        QStringLiteral("systemd"),
        QStringLiteral("dbus"),
        QStringLiteral("ny_fusion_monitor"),
    };

    const QString identity = QStringLiteral("%1 %2 %3").arg(processName, executablePath, cmdline).toLower();
    if (containsAny(identity, blacklist)) {
        return 0;
    }

    int score = 0;

    static const QStringList pathHints = {
        QStringLiteral("/steamapps/common/"),
        QStringLiteral("/steamapps/compatdata/"),
        QStringLiteral("/games/"),
        QStringLiteral("/game/"),
        QStringLiteral("/heroic/"),
        QStringLiteral("/lutris/"),
        QStringLiteral("/bottles/"),
        QStringLiteral("/gog games/"),
        QStringLiteral("/wineprefixes/"),
        QStringLiteral("\\steamapps\\common\\"),
        QStringLiteral("\\games\\"),
        QStringLiteral(".app/contents/macos/"),
        QStringLiteral("/home/"), // binários de jogos na home (lutris, etc.)
    };
    if (containsAny(identity, pathHints)) {
        score += 6;
    }

    static const QStringList runtimeHints = {
        QStringLiteral("proton"),
        QStringLiteral("wine-preloader"),
        QStringLiteral("wine64-preloader"),
        QStringLiteral("steamlaunch"),
        QStringLiteral("gamemoderun"),
        QStringLiteral("gamescope"),
        QStringLiteral("mangohud"),
        QStringLiteral("pressure-vessel"),
        QStringLiteral("reaper"),         // Steam Linux Runtime reaper
        QStringLiteral("run-in-snbox"),   // Pressure Vessel sandbox
    };
    if (containsAny(identity, runtimeHints)) {
        score += 3;
    }

    if (processName.endsWith(QStringLiteral(".exe"), Qt::CaseInsensitive)
        || executablePath.endsWith(QStringLiteral(".exe"), Qt::CaseInsensitive)) {
        score += 4;
    }

    if (processName.endsWith(QStringLiteral(".x86_64"), Qt::CaseInsensitive)
        || executablePath.endsWith(QStringLiteral(".x86_64"), Qt::CaseInsensitive)) {
        score += 5;
    }

    // Outros formatos de executáveis de jogo comuns no Linux
    if (processName.endsWith(QStringLiteral("_linux"), Qt::CaseInsensitive)
        || executablePath.endsWith(QStringLiteral("_linux"), Qt::CaseInsensitive)
        || processName.endsWith(QStringLiteral(".i386"), Qt::CaseInsensitive)
        || executablePath.endsWith(QStringLiteral(".i386"), Qt::CaseInsensitive)) {
        score += 4;
    }

    if (cmdline.contains(QStringLiteral("AppId="), Qt::CaseInsensitive)
        || cmdline.contains(QStringLiteral("SteamAppId"), Qt::CaseInsensitive)
        || cmdline.contains(QStringLiteral("steam_appid"), Qt::CaseInsensitive)) {
        score += 2;
    }

    if (rssKb >= 350000) {
        score += 2;
    } else if (rssKb >= 180000) {
        score += 1;
    }

    const QFileInfo executableInfo(executablePath);
    if (!executableInfo.suffix().isEmpty()
        && executableInfo.suffix().compare(QStringLiteral("so"), Qt::CaseInsensitive) != 0
        && executableInfo.suffix().compare(QStringLiteral("bin"), Qt::CaseInsensitive) != 0) {
        score += 1;
    }

    return score;
}

struct Candidate final {
    GameDetectionService::DetectionState state;
    qint64 rssKb{0};
};

void considerCandidate(
    Candidate& bestCandidate,
    const QString& processName,
    const QString& executablePath,
    const QString& cmdline,
    const qint64 pid,
    const qint64 rssKb,
    const std::optional<ActiveWindowInfo>& activeWindowInfo,
    const KnownGamesCatalog& catalog,
    const float gpuUsagePercent
) {
    int score = heuristicScore(processName, executablePath, cmdline, rssKb);
    const bool installedMatch = matchesInstalledGame(processName, executablePath, catalog);
    const bool activeMatch = activeWindowInfo.has_value() && activeWindowInfo->processId == pid;
    const bool fullscreenDetected = activeMatch && activeWindowInfo->fullscreen;
    const bool highGpuActivity = activeMatch && gpuUsagePercent >= 55.0f;

    if (installedMatch) {
        score += 5;
    }
    if (activeMatch) {
        score += 3;
    }
    if (fullscreenDetected) {
        score += 4;
    }
    if (highGpuActivity) {
        score += 4;
    }

    if (score < 5) {
        return;
    }

    GameDetectionService::DetectionState state;
    state.gameRunning = true;
    state.fullscreenAppDetected = fullscreenDetected;
    state.installedGameMatch = installedMatch;
    state.highGpuActivity = highGpuActivity;
    state.gameName = displayNameForCandidate(processName, executablePath, cmdline);
    state.executablePath = executablePath;
    if (activeMatch && !activeWindowInfo->title.isEmpty()) {
        state.gameName = activeWindowInfo->title;
    }
    if (installedMatch && fullscreenDetected && highGpuActivity) {
        state.detectionReason = QStringLiteral("instalado • tela cheia • GPU alta");
    } else if (fullscreenDetected && highGpuActivity) {
        state.detectionReason = QStringLiteral("tela cheia • GPU alta");
    } else if (installedMatch && fullscreenDetected) {
        state.detectionReason = QStringLiteral("instalado • tela cheia");
    } else if (installedMatch) {
        state.detectionReason = QStringLiteral("jogo instalado detectado");
    } else if (fullscreenDetected) {
        state.detectionReason = QStringLiteral("app em tela cheia");
    } else if (highGpuActivity) {
        state.detectionReason = QStringLiteral("atividade alta de GPU");
    } else {
        state.detectionReason = QStringLiteral("processo compatível");
    }
    state.processId = pid;
    state.confidenceScore = score;
    state.indexedInstalledGames = catalog.indexedGames;

    if (score > bestCandidate.state.confidenceScore
        || (score == bestCandidate.state.confidenceScore && rssKb > bestCandidate.rssKb)) {
        bestCandidate.state = state;
        bestCandidate.rssKb = rssKb;
    }
}
} // namespace

GameDetectionService::GameDetectionService(QObject* parent)
    : QObject(parent)
{
    m_pollTimer.setInterval(2500);
    connect(&m_pollTimer, &QTimer::timeout, this, &GameDetectionService::poll);
}

void GameDetectionService::start() {
    if (m_pollTimer.isActive()) {
        return;
    }

    poll();
    m_pollTimer.start();
}

void GameDetectionService::stop() {
    if (!m_pollTimer.isActive() && !m_state.gameRunning) {
        return;
    }

    m_pollTimer.stop();
    if (m_state.gameRunning || !m_state.gameName.isEmpty() || !m_state.executablePath.isEmpty()) {
        m_state = DetectionState{};
        emit detectionChanged();
    }
}

void GameDetectionService::setGpuUsagePercent(const float percent) {
    const float boundedPercent = qBound(0.0f, percent, 100.0f);
    if (qFuzzyCompare(m_lastGpuUsagePercent, boundedPercent)) {
        return;
    }

    m_lastGpuUsagePercent = boundedPercent;
    if (m_pollTimer.isActive()) {
        poll();
    }
}

bool GameDetectionService::isRunning() const {
    return m_pollTimer.isActive();
}

GameDetectionService::DetectionState GameDetectionService::currentState() const {
    return m_state;
}

void GameDetectionService::poll() {
    const DetectionState detectedState = detectGameProcess();
    if (detectedState.gameRunning == m_state.gameRunning
        && detectedState.fullscreenAppDetected == m_state.fullscreenAppDetected
        && detectedState.installedGameMatch == m_state.installedGameMatch
        && detectedState.highGpuActivity == m_state.highGpuActivity
        && detectedState.gameName == m_state.gameName
        && detectedState.executablePath == m_state.executablePath
        && detectedState.detectionReason == m_state.detectionReason
        && detectedState.processId == m_state.processId
        && detectedState.confidenceScore == m_state.confidenceScore
        && detectedState.indexedInstalledGames == m_state.indexedInstalledGames) {
        return;
    }

    m_state = detectedState;
    emit detectionChanged();
}

GameDetectionService::DetectionState GameDetectionService::detectGameProcess() const {
    const auto activeWindowInfo = detectActiveWindowInfo();
    const auto& catalog = knownGamesCatalog();
#if defined(__linux__)
    Candidate bestCandidate{};
    bestCandidate.state.indexedInstalledGames = catalog.indexedGames;
    const qint64 currentPid = QCoreApplication::applicationPid();
    const QDir procDir(QStringLiteral("/proc"));
    const QFileInfoList processEntries = procDir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot, QDir::Name);

    for (const QFileInfo& processEntry : processEntries) {
        bool pidOk = false;
        const qint64 pid = processEntry.fileName().toLongLong(&pidOk);
        if (!pidOk || pid <= 0 || pid == currentPid) {
            continue;
        }

        const QString procPath = processEntry.absoluteFilePath();
        const QString processName = readTrimmedFile(procPath + QStringLiteral("/comm"));
        const QString executablePath = QFileInfo(procPath + QStringLiteral("/exe")).symLinkTarget();
        const QString cmdline = readCmdline(procPath + QStringLiteral("/cmdline"));
        const qint64 rssKb = readMemoryRssKb(procPath + QStringLiteral("/status"));

        considerCandidate(bestCandidate, processName, executablePath, cmdline, pid, rssKb, activeWindowInfo, catalog, m_lastGpuUsagePercent);
    }

    return bestCandidate.state;
#elif defined(__APPLE__)
    Candidate bestCandidate{};
    bestCandidate.state.indexedInstalledGames = catalog.indexedGames;
    const qint64 currentPid = QCoreApplication::applicationPid();

    QProcess process;
    process.start(
        QStringLiteral("/bin/ps"),
        { QStringLiteral("-axo"), QStringLiteral("pid=,comm=,rss=,command=") }
    );
    if (!process.waitForFinished(1200)) {
        return {};
    }

    const QString output = QString::fromUtf8(process.readAllStandardOutput());
    const QStringList lines = output.split('\n', Qt::SkipEmptyParts);
    static const QRegularExpression lineRe(QStringLiteral(R"(^\s*(\d+)\s+(\S+)\s+(\d+)\s+(.*)$)"));

    for (const QString& line : lines) {
        const auto match = lineRe.match(line);
        if (!match.hasMatch()) {
            continue;
        }

        bool pidOk = false;
        const qint64 pid = match.captured(1).toLongLong(&pidOk);
        if (!pidOk || pid <= 0 || pid == currentPid) {
            continue;
        }

        const QString executablePath = match.captured(2).trimmed();
        const QString processName = QFileInfo(executablePath).fileName();
        const qint64 rssKb = match.captured(3).toLongLong();
        const QString cmdline = match.captured(4).trimmed();

        considerCandidate(bestCandidate, processName, executablePath, cmdline, pid, rssKb, activeWindowInfo, catalog, m_lastGpuUsagePercent);
    }

    return bestCandidate.state;
#elif defined(_WIN32)
    Candidate bestCandidate{};
    bestCandidate.state.indexedInstalledGames = catalog.indexedGames;
    const DWORD currentPid = static_cast<DWORD>(QCoreApplication::applicationPid());

    const HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot == INVALID_HANDLE_VALUE) {
        return {};
    }

    PROCESSENTRY32W entry{};
    entry.dwSize = sizeof(PROCESSENTRY32W);
    if (!Process32FirstW(snapshot, &entry)) {
        CloseHandle(snapshot);
        return {};
    }

    do {
        if (entry.th32ProcessID == 0 || entry.th32ProcessID == currentPid) {
            continue;
        }

        QString processName = QString::fromWCharArray(entry.szExeFile);
        QString executablePath = processName;
        qint64 rssKb = 0;

        HANDLE processHandle = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION | PROCESS_VM_READ, FALSE, entry.th32ProcessID);
        if (processHandle) {
            wchar_t pathBuffer[MAX_PATH]{};
            DWORD bufferSize = MAX_PATH;
            if (QueryFullProcessImageNameW(processHandle, 0, pathBuffer, &bufferSize)) {
                executablePath = QString::fromWCharArray(pathBuffer, static_cast<int>(bufferSize));
            }

            PROCESS_MEMORY_COUNTERS memoryCounters{};
            if (GetProcessMemoryInfo(processHandle, &memoryCounters, sizeof(memoryCounters))) {
                rssKb = static_cast<qint64>(memoryCounters.WorkingSetSize / 1024);
            }

            CloseHandle(processHandle);
        }

        considerCandidate(
            bestCandidate,
            processName,
            executablePath,
            QString(),
            static_cast<qint64>(entry.th32ProcessID),
            rssKb,
            activeWindowInfo,
            catalog,
            m_lastGpuUsagePercent
        );
    } while (Process32NextW(snapshot, &entry));

    CloseHandle(snapshot);
    return bestCandidate.state;
#else
    return {};
#endif
}

} // namespace ny::ui::services


