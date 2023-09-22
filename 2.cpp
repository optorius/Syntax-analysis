#include <iostream>
#include <initializer_list>
#include <string>
#include <algorithm>


// @brief распознаватель для следующей грамматики:
// А ::= !В!
// В ::= Т{+Т|-Т}
// Т ::= М{*М|/М}
// М ::= a|b|c|d|x|(В)
class SyntaxAnalyzer {
public: // special functions aka ctors, dtors, etc
    explicit SyntaxAnalyzer(const std::string& input)
        : input_(input)
        , index_( 0 )
    {}

    ~SyntaxAnalyzer() = default;

public: // API
    bool result() {
        //  А - задает начало
        if (!A()) {
            return false;
        }
        return input_.size() == index_;
    }

private: //
    // @brief Проверка терминала с указателем входной цепочки
    // если символы совпали, инкрементируем цепочки
    // @return true
    bool is_eq(char ch) {
        if ( index_ < input_.size() && ch == input_[ index_ ] ) {
            ++index_;
            return true;
        }
        return false;
    }

    bool A() {
        if (is_eq('!') && B() && is_eq('!')) {
            return true;
        }
        return false;
    }

    bool B() {
        if (T()) {
            // while, ибо БНФ или же форма Бэкуса
            while ( (is_eq('+') || is_eq('-')) && T() ) ;
            return true;
        }
        return false;
    }

    bool T() {
        if (M()) {
            while ( (is_eq('*') || is_eq('/')) && M() );
            return true;
        }
        return false;
    }

    bool M() {
        if (is_eq('a') || is_eq('b') || is_eq('c') || is_eq('d') || is_eq('x')) {
            return true;
        } else if (is_eq('(') && B() && is_eq(')')) {
            return true;
        }
        return false;
    }

private:
    std::string input_;
    uint16_t index_;
};

int main() try {
    auto input = {
        "!(a+b)a-c)!",
        "!ad!",
        "!(a+-c)/(a-b)!",
        "!(a+c-d)d-a!",
        "!((ax+b))x+c)x-d!",
        "d(a+b)!",
        "!a+b(c-a-b)^d",
        "!(b+bb-a*c)/(b+a)!",

        "!(a+b)*(a-c)!",
        "!a*b*c*d*d!",
        "!(a+b-c)/(a-b)!",
        "!(a+c-d)*b/d-a!",
        "!((a*x+b)*x+c)*x-d!",
        "!a/b/d*(a+b)!",
        "!a+b*(c-a-b)/d!",
        "!a*b-c*d+b/d!",
        "!(b+b*b-a*c)/(b+a)!"
    };

    std::for_each( input.begin(), input.end(),
    []( auto&& in )
    {
        std::cout << in << " ";
        std::cout << std::boolalpha << SyntaxAnalyzer(in).result() << std::endl;
    });

    return 0;
} catch( const std::exception& ex ) {
    std::cerr<<ex.what()<<std::endl;
    return 1;
}