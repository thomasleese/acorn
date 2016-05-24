- Automatic Memory Management

- Make booleantype, integertype, etc have only one possible 'creation'
- Make ::create not accept pass and nodes
- Split up type parsing in parser
- Comments
- Protocols
- Enumerate
- Arrays
- Sets
- Dictionaries
- Characters
- Strings
- Any
- Modules

- Unit tests
- Mutability
- Running code outside functions
- Channels
- Spawns
- Importing modules
- Plan units of measure
- Documentation
- GUI

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
