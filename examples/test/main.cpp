class foo
{
    virtual void helloFunc() = 0;
};

class bar : public foo
{
public:
    virtual void helloFunc() { }

};

int main(int argc, char** argv)
{
    bar* b = new bar;
    b->helloFunc();
    return 0;
}
