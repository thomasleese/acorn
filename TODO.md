- Any
- Enum
- Characters
- Strings
- File I/O
- Arrays
- Sets
- Dictionaries
- Record (change instance structure, remove new keyword)
- Modules
- Automatic Memory Management
- Unit tests
- Mutability
- Running code outside functions
- Channels
- Spawns
- Importing modules
- Plan units of measure
- Documentation
- GUI
- Maths
- Data Structures
  - Ropes
- Protocols
- Arrays of protocols
- Variables of protocols

- 10,000 LOC

- The code below must work:

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
