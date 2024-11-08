#include <qt/updater/updateinstaller.hpp>
#include <QProcess>

bool UpdateInstaller::install(const QString& downloadedUpdateFilePath)
{
	bool IsAZip = false;

	if (IsAZip)
	{
		qint64 pid;
	        QProcess::startDetached("unzip", {downloadedUpdateFilePath});
		return true;
	}

	qint64 pid;
	QProcess::startDetached("chmod", {"u+x", downloadedUpdateFilePath}); // Set Executable permissions
	QProcess::startDetached("AppImage", {downloadedUpdateFilePath}); // Run the AppImage executable file
	//QProcess::startDetached("kill", {QString::number(pid)});

	return true;
}
