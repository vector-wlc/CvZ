
// 此文件用于说明名称空间 pvz

#include "pvz.h"

// 我们先定义一个名称空间

namespace test
{

int x;
float y;
void fun()
{
    printf("我是一个函数");
}

} // namespace test

int main()
{
    // 首先使用 pvz 这个名称空间是用来包裹此框架的接口
    // 放置其与其他标识符重名
    // 其次就是为了不让用户直接使用此框架的一些接口
    // 因为那些接口不是很安全
    // 头文件 pvz.h 使用已经使用 using 语句使得 pvz 里面的一些内容直接向用户开放
    // 这些直接开放的内容是用于键控的基础接口，意思就是使用 pvz.h 里面开放的内容对于编写半自动键控程序完全够用
    // 但是如果想进一步编写全自动挂机程序，这些函数就远远不够了，所以此时介绍该框架的所有的内容，而现在正是时候。

    // 用名称空间 namespace 包裹的标识符在使用的时候必须使用相应的名称才能使用
    // 例如我们在使用 test 名称空间内的标识符的时候，必须使用 test::

    test::x = 1;     // 将名称空间 test 内的 x 变量赋值为 1
    test::y = 3, 14; // 将名称空间 test 内的 y 变量赋值为 3.14
    test::fun();     // 调用名称空间 test 内的 fun 函数

    // 如果觉得以上每次都要写 test:: 太麻烦，你也可以使用 using
    using namespace test; // 将 test 内的内容全部导入
    x = 2;
    y = 14.37;

    // 但是上述做法有个缺点，就是将 test 内的内容全部导入了，有时候我们并不想这样，所以可以写成下面这样
    using test::fun; // 直接使用 test 中的 fun     这种做法是最为推荐的

    fun(); // 调用名称空间 test 内的 fun 函数

    return 0;
}