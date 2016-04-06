- Floating point numbers
- Unsigned/Signed
- Mutablility
- Characters
- Unit tests
- Name mangling
- Arrays, tuples, strings
- Garbage Collection
- Running code outside functions
- Closures
- Channels
- Spawns
- Importing modules
- math stdlib


type Integer as Integer64

def after(n as Integer) as Channel{Void}
    let channel as Channel{Void} = Channel(buffer: 0)

    def callback() as Void
        sleep(duration: n)
        channel <- Nothing
    end

    spawn callback()

    return channel
end

def main() as Integer
    let ch as Channel{Void} = 2.after()
    <- ch  # wait for the channel
    return 0
end
