#ifndef DATASTORM_DATASTORM_H
#define DATASTORM_DATASTORM_H

inline QStringList normalizeColumnNames(const QStringList &columnNames) {
    QStringList normalizedColumnNames;
    for (const QString &columnName: columnNames) {
        QString normalizedColumnName = columnName;
        normalizedColumnName = normalizedColumnName.toLower();
        normalizedColumnName = normalizedColumnName.replace(" ", "_");
        normalizedColumnNames.append(normalizedColumnName);
    }
    return normalizedColumnNames;
}

inline  QString get_name_for_table(const QString &fileName) {
    const QStringList parts = fileName.contains("/") ? fileName.split("/") : fileName.split("\\");
    QString name = parts[parts.size() - 1];
    name = name.split(".")[0];
    name = name.toLower();
    name = name.replace(" ", "_");
    return name;
}

inline std::string getFileExtension(const std::string &filename) {
    const size_t dotPos = filename.find_last_of('.');
    if (dotPos == std::string::npos) return "";
    return filename.substr(dotPos + 1);
}

#endif //DATASTORM_DATASTORM_H
