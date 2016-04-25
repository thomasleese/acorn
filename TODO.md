- Generics
- Tuples
- Pointers & Memory Management
- Arrays
- Sets
- Dictionaries
- Characters
- Strings
- Unit tests
- Mutability
- Running code outside functions
- Closures
- Unions
- Any
- Channels
- Spawns
- Importing modules
- math stdlib


def after(n as Integer) as Channel{Void}
    let channel as Channel{Void} = Channel{Void}(buffer: 0)

    def callback() as Void
        sleep(duration: n)
        channel <- Nothing
    end

    spawn callback()

    return channel
end

let channel = 2.after()
<- channel  # wait for the channel
