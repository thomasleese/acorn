- Enum, pattern matching and (if|while) let
- 'loop', break and continue
- Match on integers, etc
- Better iterators and iterables
- Range literals

- Modules
- Characters
- Strings

- File I/O
- Arrays
- Sets
- Dictionaries
- Records (change instance structure, remove new keyword)
- Automatic Memory Management
- Unit tests
- Mutability
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
- Hindley–Milner type system

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
