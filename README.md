# proof_assistant

対話形式で定理を証明するC++プログラム。

```
g++ code.cpp
```

でコンパイルし、生成されたファイルを実行すればよい。GNU C++11で動く（はず）。警告は出る。

# 仕様

論理式は次のバッカス・ナウア記法に従う。これ以外の論理式を入力するとパースエラーを引き起こす（はず）。

```
<論理式> ::= <命題変数> | <not> <論理式> | "(" <論理式> <2項演算子> <論理式> ")"
<2項演算子> ::= <and> | <or> | <ならば>
<not> ::= "~"
<and> ::= "&"
<or> ::= "|"
<ならば> ::= "->"
<命題変数> ::= <アルファベット> | <アルファベット> <命題変数>
<アルファベット> ::= "a" | "b" | ... | "z" | "A" | "B" | ... | "Z"
```

認識されるコマンドについては起動時に表示される一覧を参照のこと。

## プロンプトについて

```
$ 
```

がプロンプトである。この左に`[CP : 論理式]`など書かれている場合があるが、これは`CP`、`OE`、`RAA`によって変更されるもので、現在示そうとしている命題を表示している。右を先頭としたスタックのように読む。

## 自動で進む証明について

証明に新しい行が書かれるたび、その論理式と`MPP`または`MTT`で導ける命題や、プロンプトの直左に書かれている`CP`等で完了するものを探し、自動で証明を追記する。

# デモ

対偶`(P->Q) |- (~Q->~P)`を証明してみる。

```
kotatsugame $ g++ a.cpp -std=c++11

kotatsugame $ ./a.out
WELCOME!
operation list:
A : Assumption
MPP : Modus Ponendo Ponens, ((A->B)&A)->B
MTT : Modus Tolendo Tolens, ((A->B)&~B)->~A
DN : Double Negation, A->~~A or ~~A->A
CP : Conditional Proof, (A|-B)->(A->B)
AI : AND-Introduction, (A,B)->(A&B)
AE : AND-Elimination, (A&B)->A or (A&B)->B
OI : OR-Introduction, A->(A|B) or B->(A|B)
OE : OR-Elimination, ((A|B), A|-C, B|-C)->C
RAA : Reductio ad Absurdum, (A|-(B&~B))->~A
del : delete the last formula [danger : DO NOT consider context]
QED : terminate this program and make proof of the last formula
help : show this message and now status

$ A
A : Assumption
Enter any formula $ P->Q                (P->Q)と括弧をつけなければならない
Invalid

$ A
A : Assumption
Enter any formula $ (P->Q)
add : 1 | (1) | (P->Q) | A
Success

$ CP
CP : Conditional Proof, (A|-B)->(A->B)
Enter formula A $ ~Q
Enter formula B $ ~P
Let's proof ~P
add : 2 | (2) | ~Q | A
add : 1,2 | (3) | ~P | (1)(2)MTT        自動で証明が進んだ
add : 1 | (4) | (~Q->~P) | (2)(3)CP     自動で証明が進んだ
add : 1,2 | (5) | ~P | (4)(2)MPP        自動で証明が進みすぎた
Success

$ del
del : delete the last formula [danger : DO NOT consider context]
1,2 | (5) | ~P | (4)(2)MPP
Success

$ QED
QED : terminate this program and make proof of the last formula

now status:
1 | (1) | (P->Q) | A
2 | (2) | ~Q | A
1,2 | (3) | ~P | (1)(2)MTT
1 | (4) | (~Q->~P) | (2)(3)CP

Proof of (P->Q) |- (~Q->~P)

1 | (1) | (P->Q) | A
2 | (2) | ~Q | A
1,2 | (3) | ~P | (1)(2)MTT
1 | (4) | (~Q->~P) | (2)(3)CP
```

# in_\*の説明

- in_CP

Conditional Proofのテスト。

- in_OE

OR-Eliminationのテスト。

- in_RAA

Reductio ad Absurdumのテスト。

- in_depend

依存行のチェックが上手くいっていることのテスト。`AI 5 4`→`AE 6 l`の操作は論理式を変えず、依存行を増やしている。
