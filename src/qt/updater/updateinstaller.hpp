#ifndef SJR_UPDATEINSTALLER_H
#define SJR_UPDATEINSTALLER_H

class QString;

namespace UpdateInstaller {

bool install(const QString& downloadedUpdateFilePath);

} // namespace UpdateInstaller

#endif