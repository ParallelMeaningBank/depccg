
#ifndef INCLUDE_TREE_H_
#define INCLUDE_TREE_H_

#include "cat.h"
#include "combinator.h"
#include <sstream>
#include <vector>
#include <memory>


namespace myccg {


class Node;
class Leaf;
class Tree;
typedef std::shared_ptr<const Node> NodeType;
typedef std::shared_ptr<const Tree> TreeType;
typedef std::shared_ptr<const Leaf> LeafType;

class FormatVisitor {
public:
    virtual int Visit(const Leaf* leaf) = 0;
    virtual int Visit(const Tree* leaf) = 0;
};

class Node
{
public:
    Node(Cat cat, const RuleType rule_type, int length)
    : cat_(cat), rule_type_(rule_type), length_(length) {}

    virtual ~Node() {}

    Cat GetCategory() { return cat_; }
    Cat GetCategory() const { return cat_; }
    const int GetLength() { return length_; }
    const int GetLength() const { return length_; }
    const std::string ToStr() const;
    virtual NodeType GetLeftChild() const = 0;
    virtual NodeType GetRightChild() const = 0;
    virtual bool IsLeaf() const = 0;
    virtual RuleType GetRuleType() { return rule_type_; }
    virtual RuleType GetRuleType() const { return rule_type_; }
    virtual const Leaf* GetHeadLeaf() const = 0;
    virtual int GetStartOfSpan() const = 0;
    virtual std::string GetWord() const = 0;
    virtual int GetHeadId() const = 0;
    virtual int GetDependencyLength() const = 0;
    virtual bool HeadIsLeft() const = 0;
    virtual bool IsUnary() const = 0;
    virtual int NumDescendants() const = 0;
    virtual int RightNumDescendants() const = 0;
    virtual int LeftNumDescendants() const = 0;
    virtual int Accept(FormatVisitor& visitor) const = 0;
    virtual const Node* GetLeftMostChild() const = 0;


    friend std::ostream& operator<<(std::ostream& ost, const Node* node) {
        ost << node->ToStr();
        return ost;
    }

protected:
    Cat cat_;
    RuleType rule_type_;
    int length_;
};
        
class Leaf: public Node
{
public:
    Leaf(const std::string& word, Cat cat, int position)
    : Node(cat, LEXICON, 1), word_(word), position_(position) {}

    ~Leaf() {}
    std::string GetWord() const { return word_; }
    const Leaf* GetHeadLeaf() const { return this; }
    bool IsLeaf() const { return true; }
    int GetPosition() const { return position_; }
    int GetHeadId() const { return position_; }
    int GetDependencyLength() const { return 0; }
    int GetStartOfSpan() const { return position_; }
    bool HeadIsLeft() const { return false; }
    bool IsUnary() const { return false; }
    int NumDescendants() const { return 0; }
    int RightNumDescendants() const { return 0; }
    int LeftNumDescendants() const { return 0; }
    NodeType GetLeftChild() const NO_IMPLEMENTATION;
    NodeType GetRightChild() const NO_IMPLEMENTATION;
    int Accept(FormatVisitor& visitor) const { return visitor.Visit(this); }
    const Node* GetLeftMostChild() const { return this; }

private:
    const std::string word_;
    const int position_;
};

RuleType GetUnaryRuleType(Cat cat);

class Tree: public Node
{
public:

    Tree(Cat cat, bool left_is_head, const Node* lchild,
            const Node* rchild, Op rule)
    : Node(cat, rule->GetRuleType(), lchild->GetLength() + rchild->GetLength()),
      left_is_head_(left_is_head),
      lchild_(lchild), rchild_(rchild), rule_(rule),
      dependency_length_(rchild_->GetHeadId() - lchild_->GetHeadId() +
            rchild_->GetDependencyLength() + lchild_->GetDependencyLength()),
      headid_(left_is_head ? lchild_->GetHeadId() : rchild_->GetHeadId()) {}

    Tree(Cat cat, bool left_is_head, NodeType lchild,
            NodeType rchild, Op rule)
    : Node(cat, rule->GetRuleType(), lchild->GetLength() + rchild->GetLength()),
      left_is_head_(left_is_head),
      lchild_(lchild), rchild_(rchild), rule_(rule),
      dependency_length_(rchild_->GetHeadId() - lchild_->GetHeadId() +
            rchild_->GetDependencyLength() + lchild_->GetDependencyLength()),
      headid_(left_is_head ? lchild_->GetHeadId() : rchild_->GetHeadId()) {}

    Tree(Cat cat, const Node* lchild)
    : Node(cat, GetUnaryRuleType(cat), lchild->GetLength()), left_is_head_(true),
      lchild_(lchild), rchild_(NULL), rule_(unary_rule),
      dependency_length_(lchild_->GetDependencyLength()),
      headid_(lchild_->GetHeadId()) {}

    Tree(Cat cat, NodeType lchild)
    : Node(cat, GetUnaryRuleType(cat), lchild->GetLength()), left_is_head_(true),
      lchild_(lchild), rchild_(NULL), rule_(unary_rule),
      dependency_length_(lchild_->GetDependencyLength()),
      headid_(lchild_->GetHeadId()) {}

    ~Tree() {}

    int GetHeadId() const { return headid_; }

    RuleType GetRuleType() const {
        if (rule_type_ == FA && *rchild_->GetCategory() == *GetCategory())
            return F_MOD;
        if (rule_type_ == BA && *lchild_->GetCategory() == *GetCategory())
            return B_MOD;
        else
            return rule_type_;
    }

    RuleType GetRuleTypeOld() const { return rule_type_; }

    int GetStartOfSpan() const {
        return lchild_->GetStartOfSpan();
    }

    bool IsLeaf() const { return false; }
    const Leaf* GetHeadLeaf() const {
        return left_is_head_ ?
            lchild_->GetHeadLeaf() : rchild_->GetHeadLeaf();
    }

    int GetDependencyLength() const { return dependency_length_; }
    bool HeadIsLeft() const { return left_is_head_; }
    bool IsUnary() const { return NULL == rchild_; }
    int NumDescendants() const {
        return ( rchild_ == NULL ? 0 : RightNumDescendants() ) + LeftNumDescendants();
    }
    std::string GetWord() const {
        if (rchild_ == NULL)
            return lchild_->GetWord();
        return lchild_->GetWord() + " " + rchild_->GetWord();
    }

    int LeftNumDescendants() const { return lchild_->NumDescendants() + 1; }
    int RightNumDescendants() const { return rchild_->NumDescendants() + 1; }
    NodeType GetLeftChild() const { return lchild_; }
    NodeType GetRightChild() const { return rchild_; }
    Op GetRule() const { return rule_; }
    int Accept(FormatVisitor& visitor) const { return visitor.Visit(this); }
    const Node* GetLeftMostChild() const { return lchild_->GetLeftMostChild(); }

private:
    bool left_is_head_;
    NodeType lchild_;
    NodeType rchild_;
    Op rule_;
    int dependency_length_;
    int headid_;
};

void ToXML(std::vector<std::shared_ptr<const Node>>&
        trees, bool feat, std::ostream& out=std::cout);

void ToXML(std::vector<const Node*>& trees, bool feat, std::ostream& out=std::cout);


class GetLeaves: public FormatVisitor {
    typedef std::vector<const Leaf*> result_type;

public:
    GetLeaves() {}
    result_type operator()(const Node* node) {
        node->Accept(*this);
        return leaves_;
    }

    int Visit(const Tree* tree) {
        tree->GetLeftChild()->Accept(*this);
        if (! tree->IsUnary())
            tree->GetRightChild()->Accept(*this);
        return 0;
    }

    int Visit(const Leaf* leaf) {
        leaves_.push_back(leaf);
        return 0;
    }

result_type leaves_;
};

class Derivation: public FormatVisitor {

public:
    Derivation(const Node* tree, bool feat=true)
        : tree_(tree), lwidth_(0), feat_(feat) { Process(); }
    Derivation(NodeType tree, bool feat=true)
        : tree_(tree.get()), lwidth_(0), feat_(feat) { Process(); }

    void Process();
    std::string Get() const { return out_.str(); }
    friend std::ostream& operator<<(std::ostream& ost, const Derivation& deriv) {
        ost << deriv.out_.str();
        return ost;
    }
    int Visit(const Tree* tree);
    int Visit(const Leaf* leaf);

private:
    const Node* tree_;
    std::stringstream out_;
    int lwidth_;
    bool feat_;
};

class AUTO: public FormatVisitor {
public:
    AUTO(const Node* tree): tree_(tree) { Process(); }
    AUTO(NodeType tree): tree_(tree.get()) { Process(); }

    void Process() { tree_->Accept(*this); }
    std::string Get() const { return out_.str(); }

    int Visit(const Tree* tree);
    int Visit(const Leaf* leaf);

private:
    const Node* tree_;
    std::stringstream out_;
};

class JaCCG: public FormatVisitor {
public:
    JaCCG(const Node* tree): tree_(tree) { Process(); }
    JaCCG(NodeType tree): tree_(tree.get()) { Process(); }

    void Process() { tree_->Accept(*this); }
    friend std::ostream& operator<<(std::ostream& ost, const JaCCG& deriv) {
        ost << deriv.out_.str();
        return ost;
    }
    std::string Get() const { return out_.str(); }

    int Visit(const Tree* tree);
    int Visit(const Leaf* leaf);

private:
    const Node* tree_;
    std::stringstream out_;
};

class XML: public FormatVisitor {

public:
    XML(const Node* tree, bool feat=true): tree_(tree), feat_(feat) { Process(); }
    XML(NodeType tree, bool feat=true): tree_(tree.get()), feat_(feat) { Process(); }

    void Process() { tree_->Accept(*this); }
    std::string Get() const { return out_.str(); }
    friend std::ostream& operator<<(std::ostream& ost, const XML& xml) {
        return ost << xml.out_.str();
    }

    int Visit(const Tree* tree);
    int Visit(const Leaf* leaf);

private:
    const Node* tree_;
    bool feat_;
    std::stringstream out_;
};

class PyXML: public FormatVisitor {

public:
    PyXML(const Node* tree, bool feat=true): tree_(tree), feat_(feat) { Process(); }
    PyXML(NodeType tree, bool feat=true): tree_(tree.get()), feat_(feat) { Process(); }

    void Process() { tree_->Accept(*this); }
    std::string Get() const { return out_.str(); }
    friend std::ostream& operator<<(std::ostream& ost, const PyXML& xml) {
        return ost << xml.out_.str();
    }

    int Visit(const Tree* tree);
    int Visit(const Leaf* leaf);

private:
    const Node* tree_;
    bool feat_;
    std::stringstream out_;
};

class CoNLL: public FormatVisitor {

public:
    CoNLL(const Node* tree);
    CoNLL(NodeType tree);
    ~CoNLL();

    void Process();
    std::string Get() const { return out_.str(); }
    friend std::ostream& operator<<(std::ostream& ost, const CoNLL& xml) {
        return ost << xml.out_.str();
    }

    int Visit(const Tree* tree);
    int Visit(const Leaf* leaf);

private:
    int id_;
    const Node* tree_;
    int length_;
    int* heads_;
    const Leaf** leaves_;
    std::stringstream out_;
};

} // namespace myccg

#endif
