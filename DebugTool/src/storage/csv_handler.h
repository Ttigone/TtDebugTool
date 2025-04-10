#ifndef STORAGE_CSV_HANLDER_H
#define STORAGE_CSV_HANLDER_H

namespace Storage {

class TtCsvHandler {
 public:
  TtCsvHandler();

  static QVector<QStringList> readCsv(const QString& filePath,
                                      bool hasHeader = true) {
    QVector<QStringList> data;
    QFile file(filePath);

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
      return data;

    QTextStream in(&file);

    // 处理标题行
    if (hasHeader && !in.atEnd()) {
      QString headerLine = in.readLine();
      headerFields = headerLine.split(",");
    }

    // 读取数据行
    while (!in.atEnd()) {
      QString line = in.readLine();
      QStringList fields = parseCsvLine(line);  // 使用更健壮的解析
      data.append(fields);
    }

    file.close();
    return data;
  }

  // 写入二维字符串数组到CSV文件
  static bool writeCsv(const QString& filePath,
                       const QVector<QStringList>& data,
                       const QStringList& headers = QStringList()) {
    QFile file(filePath);

    if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
      return false;

    QTextStream out(&file);

    // 写入标题行
    if (!headers.isEmpty()) {
      QStringList escapedHeaders;
      for (const QString& header : headers) {
        escapedHeaders << escapeCsvField(header);
      }
      out << escapedHeaders.join(",") << "\n";
    }

    // 写入数据行
    for (const QStringList& row : data) {
      QStringList escapedFields;
      for (const QString& field : row) {
        escapedFields << escapeCsvField(field);
      }
      out << escapedFields.join(",") << "\n";
    }

    file.close();
    return true;
  }

  // 获取标题行
  static QStringList headers() { return headerFields; }

 private:
  static QStringList headerFields;

  // 更健壮的CSV行解析，处理引号和逗号
  static QStringList parseCsvLine(const QString& line) {
    QStringList fields;
    bool inQuote = false;
    QString field;

    for (int i = 0; i < line.length(); ++i) {
      QChar c = line[i];

      if (c == '"') {
        if (i + 1 < line.length() && line[i + 1] == '"') {
          // 双引号转义为单引号
          field += '"';
          ++i;
        } else {
          // 切换引号状态
          inQuote = !inQuote;
        }
      } else if (c == ',' && !inQuote) {
        // 字段分隔符
        fields.append(field);
        field.clear();
      } else {
        field += c;
      }
    }

    // 添加最后一个字段
    fields.append(field);

    return fields;
  }

  // 转义CSV字段，处理特殊字符
  static QString escapeCsvField(const QString& field) {
    if (field.contains(',') || field.contains('"') || field.contains('\n')) {
      // 需要用引号包围
      QString escaped = field;
      escaped.replace("\"", "\"\"");  // 将引号替换为双引号
      return "\"" + escaped + "\"";
    }
    return field;
  }
};

// 静态成员初始化
QStringList TtCsvHandler::headerFields;

}  // namespace Storage

#endif  // STORAGE_CSV_HANLDER_H
