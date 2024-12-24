#include <iostream>
#include <string>
#include <vector>
#include <memory>
#include <regex>
#include <fstream> 

// Базовый интерфейс
class ICodeToHTML {
public:
    virtual std::string convert(const std::string& code) = 0;
    virtual ~ICodeToHTML() = default;
};

// Базовое преобразование
class BasicCodeToHTML : public ICodeToHTML {
public:
    std::string convert(const std::string& code) override {
        // Преобразуем код в HTML с сохранением форматирования
        std::string html = "<pre><code>";

        for (const char& c : code) {
            if (c == '<') {
                html += "&lt;";
            } else if (c == '>') {
                html += "&gt;";
            } else {
                html += c;
            }
        }

        html += "</code></pre>";
        return html;
    }
};

// Базовый декоратор
class CodeToHTMLDecorator : public ICodeToHTML {
protected:
    std::shared_ptr<ICodeToHTML> wrappedConverter;

public:
    explicit CodeToHTMLDecorator(std::shared_ptr<ICodeToHTML> converter)
        : wrappedConverter(std::move(converter)) {}

    std::string convert(const std::string& code) override {
        return wrappedConverter->convert(code);
    }
};

// Декоратор: выделение ключевых слов
class KeywordHighlighter : public CodeToHTMLDecorator {
    std::vector<std::string> keywords;

public:
    explicit KeywordHighlighter(std::shared_ptr<ICodeToHTML> converter)
        : CodeToHTMLDecorator(std::move(converter)) {
        keywords = {"int", "return", "if", "else", "for", "while", "class", "public", "private", "void"};
    }

    std::string convert(const std::string& code) override {
        std::string html = wrappedConverter->convert(code);

        // Выделяем ключевые слова с помощью regex
        for (const auto& keyword : keywords) {
            std::regex keywordRegex("\\b" + keyword + "\\b");
            html = std::regex_replace(html, keywordRegex, "<span style='color: blue; font-weight: bold;'>" + keyword + "</span>");
        }

        return html;
    }
};

// Декоратор: выделение строковых литералов
class StringLiteralHighlighter : public CodeToHTMLDecorator {
public:
    explicit StringLiteralHighlighter(std::shared_ptr<ICodeToHTML> converter)
        : CodeToHTMLDecorator(std::move(converter)) {}

    std::string convert(const std::string& code) override {
        std::string html = wrappedConverter->convert(code);

        // Выделяем строковые литералы
        std::regex stringRegex("\"(.*?)\"");
        html = std::regex_replace(html, stringRegex, "<span style='color: green;'>\"$1\"</span>");

        return html;
    }
};

// Декоратор: выделение комментариев
class CommentHighlighter : public CodeToHTMLDecorator {
public:
    explicit CommentHighlighter(std::shared_ptr<ICodeToHTML> converter)
        : CodeToHTMLDecorator(std::move(converter)) {}

    std::string convert(const std::string& code) override {
        std::string html = wrappedConverter->convert(code);

        std::regex singleLineCommentRegex("//(.*?)\\n");
        html = std::regex_replace(html, singleLineCommentRegex, "<span style='color: gray;'>//$1</span>\n");

        std::regex multiLineCommentRegex("/\\*([\\s\\S]*?)\\*/");
        html = std::regex_replace(html, multiLineCommentRegex, "<span style='color: gray;'>/*$1*/</span>");

        return html;
    }
};

int main() {
    std::string cppCode = R"(#include <iostream>
int main() {
    /*Пример программы
    с мультистрочными комментариями
    !!!*/
    std::string text = "Hello, world!";
    std::cout << text << std::endl;
    return 0;
    return 1;
})";

    // Базовое преобразование
    auto basicConverter = std::make_shared<BasicCodeToHTML>();

    // Добавляем декораторы
    auto keywordHighlighter = std::make_shared<KeywordHighlighter>(basicConverter);
    auto stringHighlighter = std::make_shared<StringLiteralHighlighter>(keywordHighlighter);
    auto commentHighlighter = std::make_shared<CommentHighlighter>(stringHighlighter);

    // Конечный результат
    std::string html = commentHighlighter->convert(cppCode);

    // Сохраняем результат в HTML файл
    std::ofstream outFile("output.html");
    if (outFile.is_open()) {
        outFile << "<html><body>" << std::endl;
        outFile << "<h1>Formatted C++ Code</h1>" << std::endl;
        outFile << html << std::endl;
        outFile << "</body></html>" << std::endl;
        outFile.close();
        std::cout << "HTML файл успешно сохранён как output.html" << std::endl;
    } else {
        std::cerr << "Ошибка при открытии файла для записи!" << std::endl;
    }

    return 0;
}