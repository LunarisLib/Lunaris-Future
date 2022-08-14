# Lunaris-Future

*Future is a needed feature in C++ for asynchronous tasks. This tries to perform exactly that.*

*Standard `std::future` lack the `then()` part and the `get()` holding the value for as long as you need. You can `get_take()` if you want to move the stored value, but that's a choice, and that's what's great about it.*
*The `promise` and `future`s here are all valid for as long as they exist. This means you can actually use it as a tunnel between threads like messages of a specific type all the time.*
*This can be accomplished by doing `get_take()` and waiting for the next value. It won't hold more than one value at a time, so it's not exactly designed for that, but it can work (last example).*

## *An example:*
```cpp
using namespace Lunaris;

promise<int> prom;
future<int> nxt = prom.get_future()
	.then([](int val) { std::cout << "Thenn!: " << val << "\n"; return val * 2; })
	.then([](int val) { std::cout << "Then now return void, but got: " << val << "\n"; return; })
	.then([]() { std::cout << "Then void to void.\n"; return; })
	.then([]() { std::cout << "Then got void, but returning 10 again\n"; return 10; });

std::thread thr([&] {
	nxt.wait();
	std::cout << "Got value, wait 1 sec...\n";
	std::this_thread::sleep_for(std::chrono::seconds(1));
	std::cout << "Value set: " << nxt.get() << std::endl;
});

std::cout << "Wait...\n";
std::this_thread::sleep_for(std::chrono::seconds(1));

prom.set_value(64);
thr.join();
```

#### *How does it work?*

*A `promise` of type `int` can create a future of its type. A `future` holds information about a value that is (probably) not set yet.*
*From a `future` of a type `T` you can add a function tied to it via `then()`. The function will replace any `get()` from it, and it is executed exactly when (and where) the `promise` (or `future`, if it's `then()` of `then()`) is set.*
*Using a function you can then get a `future` for the return of it, like, `[](T a){/*do smth and return type X;*/}` will create a `future` of type `X`.*

*The code above uses all the possibilites: type to type, type to void, void to void and void to type. This proves that any type will work.*

##### *Explaining step by step:*
- *A `promise` of type `int` is created. This is used by your async task to set the value in the future.*
- *After that, we get the `future` using `get_future()`, and from that `future` of type `int` we hook a function that handles that value directly, showing `Thenn!: <number>` and returning `val * 2`. It's a `future` of `int` again (return from `then()`) because of that return.*
- *The next `future` is set with another `then()` directly, printing another message and returning nothing. That's `void`. A `future` of `void` is created after that.*
- *That `future` is then set with a function that gets nothing (`void`) and returns nothing too, another `future` of `void`.*
- *Lastly we have a function that gets nothing (`void`) and returns an `int`. A `future` of `int` is created.*
- *The thread `thr` waits for the `nxt` (last `future` of `int`) to be set. It will be only set after **all the other `future`s, in sequence**, so that means all those `future`s with `then()` run and set in sequence one after another.*
- *After one second, the main thread runs `set_value(64)`, triggering the `future`s in **sequence**. All functions are called in sequence exactly there and the last value is set for the last `future`, `nxt`.*
- *The thread `thr` detects the value and counts 1 second before showing the last value, `10`.*

#### *Expected result:*
```
Wait...
Thenn!: 64
Then now return void, but got: 128
Then void to void.
Then got void, but returning 10 again
Got value, wait 1 sec...
Value set: 10
```

*You can actually call the `promise` as many times as you want. This enables it to be used in some weird ways.*

## *Using it as a messager between threads:*
***Not recommended, there are better ways, but you can, if you really want to.***
```cpp
using namespace Lunaris;

promise<int> pa, pb;
future<int> fa, fb;

fa = pa.get_future();
fb = pb.get_future();

std::thread a([&] {
	pa.set_value(0); // start
	while (fb.get() < 10) {
		std::cout << "ThreadA: " << fb.get() << std::endl;
		pa.set_value(fb.get_take() + 1);
	}
	pa.set_value(10);
});
std::thread b([&] {
	while (fa.get() < 10) { // waits for a
		std::cout << "ThreadB: " << fa.get() << std::endl;
		pb.set_value(fa.get_take() + 1);
	}
	pb.set_value(10);
});

a.join();
b.join();
```

*This works by design, not because it was expected to be used that way. "You can do it" doesn't mean you should, but I'll show it just so I can say that this works.*

#### *What is this, you ask?*

*Two `promise`s, two `future`s, two threads. One thread starts with `set_value()` and waits the second via `get()`, then the second gets the value set and set that value + 1. The first `get_take()` the value + 1 and then sets that + 1...*
*This goes until one of them gets 10, then both set 10 for convenience and exit. This is a counter using two threads and "infinite" `promise`s.*

*What this means is that you can have a kind of tunnel between two tasks and maybe use that to ask something and wait it back wihtout recreating the `future`s and `promise`s, or maybe you want a fancy combo of functions tied to one argument and trigger that as many times as you want. This can be useful, but at the same time this is not fast at all (wait, set, wait, get, ...).*


