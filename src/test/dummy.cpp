class X
{
    public:
    int x;
    X(int _x) : x{_x}{}
    X operator&(const X& _x)
    {
        return x & _x.x;
    }

};

int main(void)
{

    X x = 0;

    0x3b;

    return 0;
}