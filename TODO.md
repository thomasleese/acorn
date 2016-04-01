- Running code outside functions
- Garbage Collection
- Closures
- Arrays
- Channels
- Spawns
- Tuples
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