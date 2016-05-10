- Merge symbol table builder with parser
- Use numbers with metadata as mangling instead
- Implement my own assert macro
- Proper generics

- Pointers, Memory Management and Garbage Collection

- Plan units of measure
- Documentation

- Arrays
- Tuples
- Sets
- Dictionaries
- Characters
- Strings
- Unions
- Any
- Generics

- Unit tests
- Mutability
- Running code outside functions
- Channels
- Spawns
- Importing modules

- Maths library

- Data Structures library
  - Ropes

def after(n as Integer) as Channel{Void}
    let channel = new Channel{Void}(buffer: 0)

    def callback() as Void
        sleep(duration: n)
        channel <- Nothing
    end

    spawn callback()

    return channel
end

let channel = 2.after()
<- channel  # wait for the channel
