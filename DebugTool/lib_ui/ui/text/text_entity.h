/*****************************************************************//**
 * \file   text_entity.h
 * \brief  文本属性
 * 
 * \author C3H3_Ttigone
 * \date   August 2024
 *********************************************************************/

#ifndef UI_TEXT_ENTITY_H
#define UI_TEXT_ENTITY_H

#include <QClipboard>

enum class EntityType : uchar {
  Invalid = 0,

  Url,
  CustomUrl,
  Email,
  Hashtag,
  Cashtag,
  Mention,
  MentionName,
  CustomEmoji,
  BotCommand,
  MediaTimestamp,
  Colorized,  // Senders in chat list, attachments in chat list, etc.
  Phone,

  Bold,
  Semibold,
  Italic,
  Underline,
  StrikeOut,
  Code,  // inline
  Pre,   // block
  Blockquote,
  Spoiler,
};


class EntityInText;
using EntitiesInText = QVector<EntityInText>;

class EntityInText {

};



struct TextWithEntities {
  QString text;
  EntitiesInText entities;

  bool empty() const { return text.isEmpty(); }

  /// @brief 翻转文本
  /// @param size
  /// @param entitiesCount
  void reserve(int size, int entitiesCount = 0) {
    text.reserve(size);
    entities.reserve(entitiesCount);
  }

  /// @brief 追加文本
  /// @param other
  /// @return
  // TextWithEntities& append(TextWithEntities&& other) {}

  // TextWithEntities& append(const TextWithEntities& other) {}

  // TextWithEntities& append(const QString& other) {}

  // TextWithEntities& append(const QLatin1String& other) {}

  // TextWithEntities& append(const QChar& other) {
  //   // return
  // }
};

struct TextForMimeData
{
  QString expanded;
  TextWithEntities rich;

  bool empty() const {
    return expanded.isEmpty();
  }

  void reserve(int size, int entitiesCount = 0) {
		expanded.reserve(size);
		rich.reserve(size, entitiesCount);
	}


};

struct TextWithTags {
  struct Tag {
    int offset = 0;
    int length = 0;
    QString id;

    //friend inline auto operator< = >(const Tag &, const Tag &) = default;
    //friend inline bool operator==(const Tag &, const Tag &) = default;
  };
  using Tags = QVector<Tag>;

  QString text;
  Tags tags;

  [[nodiscard]] bool empty() const { return text.isEmpty(); }
  //friend inline auto operator<=>(const TextWithTags &,
  //                               const TextWithTags &) = default;
  //friend inline bool operator==(const TextWithTags &,
  //                              const TextWithTags &) = default;
};

enum {
  TextParseMultiline = 0x001,
  TextParseLinks = 0x002,
  TextParseMentions = 0x004,
  TextParseHashtags = 0x008,
  TextParseBotCommands = 0x010,
  TextParseMarkdown = 0x020,
  TextParseColorized = 0x040,
};


namespace TextUtilities {

bool IsValidProtocol(const QString &protocol);
bool IsValidTopDomain(const QString &domain);

const QRegularExpression &RegExpMailNameAtEnd();
const QRegularExpression &RegExpHashtag();
const QRegularExpression &RegExpHashtagExclude();
const QRegularExpression &RegExpMention();
const QRegularExpression &RegExpBotCommand();
const QRegularExpression &RegExpDigitsExclude();
QString MarkdownBoldGoodBefore();
QString MarkdownBoldBadAfter();
QString MarkdownItalicGoodBefore();
QString MarkdownItalicBadAfter();
QString MarkdownStrikeOutGoodBefore();
QString MarkdownStrikeOutBadAfter();
QString MarkdownCodeGoodBefore();
QString MarkdownCodeBadAfter();
QString MarkdownPreGoodBefore();
QString MarkdownPreBadAfter();
QString MarkdownSpoilerGoodBefore();
QString MarkdownSpoilerBadAfter();

// Text preprocess.
QString EscapeForRichParsing(const QString &text);
QString SingleLine(const QString &text);
TextWithEntities SingleLine(const TextWithEntities &text);
QString RemoveAccents(const QString &text);
QString RemoveEmoji(const QString &text);
QStringList PrepareSearchWords(
    const QString &query, const QRegularExpression *SplitterOverride = nullptr);
bool CutPart(TextWithEntities &sending, TextWithEntities &left, int limit);

struct MentionNameFields {
  uint64 selfId = 0;
  uint64 userId = 0;
  uint64 accessHash = 0;
};
[[nodiscard]] MentionNameFields MentionNameDataToFields(QStringView data);
[[nodiscard]] QString MentionNameDataFromFields(
    const MentionNameFields &fields);

// New entities are added to the ones that are already in result.
// Changes text if (flags & TextParseMarkdown).
TextWithEntities ParseEntities(const QString &text, int32 flags);
void ParseEntities(TextWithEntities &result, int32 flags);

void PrepareForSending(TextWithEntities &result, int32 flags);
void Trim(TextWithEntities &result);

enum class PrepareTextOption {
  IgnoreLinks,
  CheckLinks,
};
inline QString PrepareForSending(
    const QString &text,
    PrepareTextOption option = PrepareTextOption::IgnoreLinks) {
  auto result = TextWithEntities{text};
  auto prepareFlags = (option == PrepareTextOption::CheckLinks)
                          ? (TextParseLinks | TextParseMentions |
                             TextParseHashtags | TextParseBotCommands)
                          : 0;
  PrepareForSending(result, prepareFlags);
  return result.text;
}

// Replace bad symbols with space and remove '\r'.
void ApplyServerCleaning(TextWithEntities &result);

[[nodiscard]] int SerializeTagsSize(const TextWithTags::Tags &tags);
[[nodiscard]] QByteArray SerializeTags(const TextWithTags::Tags &tags);
[[nodiscard]] TextWithTags::Tags DeserializeTags(QByteArray data,
                                                 int textLength);
[[nodiscard]] QString TagsMimeType();
[[nodiscard]] QString TagsTextMimeType();

inline const auto kMentionTagStart = QLatin1String("mention://");

[[nodiscard]] bool IsMentionLink(QStringView link);
[[nodiscard]] QString MentionEntityData(QStringView link);
[[nodiscard]] bool IsSeparateTag(QStringView tag);
[[nodiscard]] QString JoinTag(const QList<QStringView> &list);
[[nodiscard]] QList<QStringView> SplitTags(QStringView tag);
[[nodiscard]] QString TagWithRemoved(const QString &tag,
                                     const QString &removed);
[[nodiscard]] QString TagWithAdded(const QString &tag, const QString &added);
[[nodiscard]] TextWithTags::Tags SimplifyTags(TextWithTags::Tags tags);

EntitiesInText ConvertTextTagsToEntities(const TextWithTags::Tags &tags);
TextWithTags::Tags ConvertEntitiesToTextTags(const EntitiesInText &entities);
std::unique_ptr<QMimeData> MimeDataFromText(const TextForMimeData &text);
std::unique_ptr<QMimeData> MimeDataFromText(TextWithTags &&text);
void SetClipboardText(const TextForMimeData &text,
                      QClipboard::Mode mode = QClipboard::Clipboard);

}  // namespace TextUtilities

#endif  // UI_TEXT_ENTITY_H
