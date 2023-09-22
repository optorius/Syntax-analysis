#include <iostream>
#include <stack>
#include <algorithm>
#include <vector>
#include <map>
#include <cassert>
#include <optional>
#include <sstream>

struct grammar {
    std::string nonterminal;
    std::string terminal;
    char start;
    struct rule {
        char left;
        std::string right;
    };
    std::vector< rule > rules;
};
/*
            {'A', "!B!"},
            {'B', "T{+T|-T}"},
            {'T', "M{*M|/M}"},
            {'M', "a|b|c|d|x|(B)"}
        "!(a+b)*c"
*/

namespace
{

bool inStr( char check, const std::string& in )
{
    auto ret =  std::any_of(
        in.cbegin(),
        in.cend(),
        [&]( auto&& elem ) {
            return elem == check;
        }
    );

#ifdef god_mode
    std::cout << std::boolalpha << check << " in " << in << ": " << ret
        << std::endl;
#endif
    return ret;
}

int32_t getIndexOfRule( char notterminal, const std::vector< grammar::rule> & rules, int32_t count = 1)
{
    auto tmp = 0;
    auto it = std::find_if( rules.cbegin(), rules.cend(), [&]( auto&& elem ) {
        return elem.left == notterminal && ++tmp == count;
    });
    if ( it == rules.end() ) {
        return -1;
    }

    return std::distance( rules.begin(), it );
}

std::string getRule( char notterminal, const std::vector< grammar::rule >& rules, int32_t count = 1 )
{
    auto tmp = 0;
    auto it = std::find_if( rules.cbegin(), rules.cend(), [&]( auto&& elem ) {
        return elem.left == notterminal && ++tmp == count;
    });
    if ( it == rules.end() ) {
        return {};
    }
    return it->right;
}

} // namespace

// } // namespace grammar

class Parser {
public: // ctors
    Parser( const grammar& grammar, const std::string& input )
        : result_{}
        , grammar_ ( grammar )
        , state_ ( state::q )
        , index_of_input_ ( 0 ) {
        l2.push( grammar_.start );
        while ( true )
        {

#ifdef god_mode
        l1_print();
        l2_print();
        std::cout << "w: " << input[index_of_input_] << std::endl;
#endif
        switch( state_ )
        {
            case state::q:
            {
                if ( inStr( l2.top(), grammar_.nonterminal ) )
                {
                    // активная вершина - терминал
                    first_step();
                }
                else if ( l2.top() == input[index_of_input_] )
                {
                    // входной символ совпал с порожденным
                    second_step();
                    if ( l2.empty() && index_of_input_ >= input.size() ) {
                        state_ = state::t;
                    }
                    else if ( l2.empty() || index_of_input_ >= input.size() ) {
                        third_step();
                    }
                }
                else
                {
                    // неудачное сравнение входного символа с порожденным
                    fourth_step();
                }

                break;
            }
            case state::b:
            {
                if ( inStr( l1.top().symbol, grammar_.terminal ))
                {
                    // возрат по терминау
                    fifth_step();
                }
                else if (
                    !getRule( l1.top().symbol, grammar_.rules, l1.top().alternative_index + 1 ).empty()
                )
                {
                    // возрат по нетерминалу, существует альтернатива
                    sixthA_step();
                    state_ = state::q;
                }
                else if ( index_of_input_ == 0 &&  l1.top().symbol == grammar_.start )
                {
                    // перебраны все альтернативы, следовательно вывода для входной цепочки не нашлось
                    sixthB_step();
                    return;
                }
                else {
                    // все альтернативы у нетерминала были рассмотрены, выполняется возрат этого нетерминала
                    sixthC_step();
                }
                break;
            }
            case state::t:
            {
                // успешное завершение
                third_finish_step();
                return;
            }
        }
#ifdef god_mode
        l1_print();
		l2_print();

        std::cout << "\n\n===========\n";
#endif
        }
    }

public: // methods
    /// @return some output
    const std::string& get_result() const{
        return result_;
    }

private: // internal methods
    /// @brief состояния
    enum class state {
                // состояние
        q = 0,          //   нормальной работы
        b = 1,          //   отката
        t = 2           //   нормального завершения
    };

    void l1_print() {
        auto copy = l1;
        std::cout << "l1:\nsymbols: ";
        while ( !copy.empty() ) {
            auto top = copy.top();
            std::cout << top.symbol << "|";
            copy.pop();
        }
        std::cout << "\nindexes: ";
        copy = l1;
        while ( !copy.empty() ) {
            auto top = copy.top();
            std::cout << top.alternative_index << "|";
            copy.pop();
        }
        std::cout << std::endl;
    }

    void l2_print() {
        auto copy = l2;
        std::cout << "l2: ";
        while ( !copy.empty() ) {
            auto top = copy.top();
            std::cout << top;
            copy.pop();
        }
        std::cout << std::endl;
    }

    void first_step() {
        l1.push( { l2.top(), 1 } );

        auto rule = getRule( l2.top(), grammar_.rules );

        auto count = 0;
        l2.pop();
        std::for_each( rule.rbegin(), rule.rend(), [&] ( auto&& ch ) {
            l2.push( ch );
        });
#ifdef god_mode
        std::cout << "It's a first step\n";
        l1_print();
        l2_print();
#endif
    }

    /// @brief pushing to l1
    void second_step() {
        l1.push( { l2.top(), -1 });
        l2.pop();
        ++index_of_input_;
#ifdef god_mode
        std::cout << "It's a second step\n";
        l1_print();
        l2_print();
#endif
    }

    /// @brief output to result
    void third_finish_step() {
        auto copy = l1;
        auto out = std::ostringstream{};
        while ( !copy.empty() ) {
            if ( inStr( copy.top().symbol, grammar_.nonterminal ) ) {
                out << ( getIndexOfRule( copy.top().symbol, grammar_.rules, copy.top().alternative_index ) + 1 ) << ' ';
            }
            copy.pop();
        }
        auto tmp = out.str();
        result_ = { tmp.rbegin(), tmp.rend() };
#ifdef god_mode
        std::cout << "third finish step\n";
#endif
    }
    /// @brief  set to state b
    void third_step() {
        state_ = state::b;
#ifdef god_mode
        std::cout << "third step\n";
        l1_print();
        l2_print();
#endif
    }

    /// @brief  set to state b
    void fourth_step() {
        state_ = state::b;
#ifdef god_mode
        std::cout << "fourth_step\n";
        l1_print();
        l2_print();
#endif
    }

    /// @brief Возврат по терминалу
    /// терминал из l1 переносится в l2,
    /// указатель входной цепочки уменьшается
    void fifth_step() {
        l2.push( l1.top().symbol );
        l1.pop();
        --index_of_input_;
#ifdef god_mode
        std::cout << "fifth_step\n";
        l1_print();
        l2_print();
#endif
    }

    void sixthA_step() {

        auto count = 0;
        while ( count++ < getRule( l1.top().symbol, grammar_.rules, l1.top().alternative_index ).size() ) {
            l2.pop();
        }

        ++l1.top().alternative_index;
        auto rule = getRule( l1.top().symbol, grammar_.rules, l1.top().alternative_index );
        std::for_each( rule.rbegin(), rule.rend(), [&] ( auto&& ch ) {
            l2.push( ch );
        });
#ifdef god_mode
        std::cout << "sixthA_step\n";
        l1_print();
        l2_print();
#endif
    }

    void sixthB_step() {
        result_ = "error";
    }

    void sixthC_step() {
        auto count = 0;
        while ( count++ < getRule( l1.top().symbol, grammar_.rules, l1.top().alternative_index ).size() ) {
            l2.pop();
        }
        l2.push ( l1.top().symbol );
        l1.pop();
#ifdef god_mode
        std::cout << "sixthC_step\n";
        l1_print();
        l2_print();
#endif
    }

private: // internal fields
    struct alpha
    {
        char symbol;
        int32_t alternative_index;
    };

private:
    std::string result_;

private:
    // our grammar boy
    grammar grammar_;

private:
    state state_;

    int32_t index_of_input_;

    // keeps alpha, which keeps index of rule and last reader character
    std::stack<alpha> l1;

    // keeps output
    std::stack<char> l2;
};

// 2 - задание
// метод рекурсвиного спуска
// уже дана расширенная граммтика но будет отличие - список номерров правил можно не строить
// подаете на вход стршку - результат правильная ли строка или она ошибочная

int main()
try
{

#ifndef REAL

    Parser checker { {
        "BTM",  // nonterminal
        "+*ab", // terminal
        'B',        // start character
                    // rules
        {
            { 'B', "T+B" },
            { 'B', "T" },
            { 'T', "M" },
            { 'T', "M*T" },
            { 'M', "a" },
            { 'M', "b" }
        }
        },
        "a+b"
    };
    std::cout << "Result: " << checker.get_result() << std::endl;
#else
    std::cout << "Result: " << Parser{ {
        "ABTM", // nonterminal
        "!+*()ab",  // terminal
        'A',        // start character
                    // rules
        {
            { 'A', "!B!" }, // 1
            { 'B', "T" },   // 2
            { 'B', "T+B" }, // 3
            { 'T', "M" },   // 4
            { 'T', "M*T" }, // 5
            { 'M', "a" },   // 6
            { 'M', "b" },   // 7
            { 'M', "(B)" } // 8
        }
        },
        "!(a+b)*a+b*(a+b)!"
    }.get_result() << std::endl;

#endif
/*
    // I - индекс альтернатив нетерминалов
    // s { q, b, t }
    // q - состнояние нормальной работы, t - завершение, b - состояние возрата
    // i - значение указателя входной цепочки омега ( указатель входной цепочки )
    // L1 будет хранить историю проделанных подстановок
    // - индексы алльтернаив и прочитанные символы a_i ( alpha )
    // стек L2 - хранит текущую цепочку вывода ( заносится правая часть правила y_i )
    // y - альтернатива
    // - a ( alpha ) - содержимое L1, b ( beta ) - содержимое L2

    // применяется первая альтернатива
    // ( q, i, a, Ab ) |= ( q, i, aA1, y1b)
    // step 1
*/
    return 0;
}
catch ( const std::exception & e )
{
    std::cerr << e.what() << std::endl;
    return 1;
}
catch ( ... )
{
    std::cerr << "Unknown exception" << std::endl;
    return 1;
}
