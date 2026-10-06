## 1. Three categories, and what other languages pay for them

### Strings: Java Strings

- Java's String also keeps its length as a field, handles NULL characters, and is garbage collected. What it pays is immutability. Our dt_str_append mutates in place and doubles capacity when full. In Java, s += x builds a new string and copies everything each time, so assembling a long string in a loop is O(n²).

### Python: Dictionary

- Python's dictionary is usually much more memory-hungry, while your C map can be much more memory-efficient and faster for raw operations—but Python gives you far more safety and convenience.

### Python: Integers

- In our code the values are stored directly as long long, which is a fixed-size integer. our code also explicitly checks for overflow before performing the operation. Python gives you arbitrary-size integers automatically. So instead of manually checking whether an operation would exceed LLONG_MAX or LLONG_MIN, Python simply grows the integer when necessary. Python integers use more memory and arithmetic is generally slower than C's fixed-size long long. Python has to store an integer as an object with additional information such as its type and representation. A C long long, by comparison, is just a fixed-size value. This is noticeable once you start doing calculations that go into the millions. Python’s integers would use more memory and be slower compared to C’s integers.

## 2. The hand-written tag check versus a compiler-enforced one

- C gives you more freedom since the compiler doesn’t enforce that the tag and the union member match. 

```bash
dt_value v = dt_value_str("hello"); 
long long x = v.as.integer; 
```

Is a valid code even though v.tag says DT_STR. It is the programmer’s responsibility to check the tag first. While other languages like Rust with an enforced tagged union, the compiler makes the relationship part of the type system. Where the compiler can reject the other possible variants of a value. So the compiler provides guarantee that C code does not automatically provide, safe access where you can’t accidentally access the wrong variant’s data. C’s freedom is worth wanting for a low-level programming language where you can directly control memory allocation, interoperation with hardware, unusual representation that doesn’t fit neatly into a language’s type system, and performance sensitive code. But for a tagged union like dt_value, it is mostly a liability because it makes type-safety and exhaustive case handling the programmer's responsibility. Rust, ML, and Swift make these checks part of the type system, preventing invalid variant access and requiring all cases to be handled.

## 3. dt_map without a separate insertion order

- An alternative design would be to remove the separate order structure and store entries only in the hash buckets. This would reduce memory usage and simplify the implementation because every entry would exist in only one linked list, and lookup, insertion, and deletion could still remain O(1) on average. However, removing order would break the map's ability to preserve insertion order. Functions such as dt_map_key_at() would no longer be able to reliably return the first, second, or third key that was inserted, since the bucket arrangement is based on hash values rather than insertion order. I would only ship this design if insertion order were not part of the map's requirements; for this assignment, I would keep order because stable output and dt_map_key_at() explicitly depend on it.

## 4. Access after release versus an unreleased allocation

- Access after release is more dangerous than an unreleased allocation. In access after release the program uses memory that has already been freed. That memory might now belong to something else, so the program could read incorrect or private data, change someone else's data, crash, or even create a security vulnerability. While an unreleased allocation (memory leak), the program forgets to free memory. The program still works correctly, but memory usage keeps increasing. In a long-running server, repeated leaks can eventually use all available memory and cause the server to slow down or crash. For a command-line program that only runs for a second, a memory leak is usually not a big problem because the operating system frees the program's memory when it exits. However, accessing released memory is still dangerous, because it can cause wrong results, crashes, or corrupted files even during that short time. Our dt_ref_release frees the cell, sets p->cell = NULL and sets released = true. dt_ref_borrow and dt_ref_release check the flag before touching the cell, and dt_ref_is_released lets the driver sweep for leaks at exit.
