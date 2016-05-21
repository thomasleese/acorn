- Automatic Memory Management

- Merge symbol table builder with parser
- Use numbers with metadata as mangling instead

- Design types better (try to remove constructors?)
- Enumerate/Range
- Arrays
- Sets
- Dictionaries
- Characters
- Strings
- Any

- Unit tests
- Mutability
- Running code outside functions
- Channels
- Spawns
- Importing modules
- Plan units of measure
- Documentation

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
