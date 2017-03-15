- Support passing functions around.
- Implement full generics.
- Code improvements, with tests.



- Pattern matching
- Remove 'if let'
- 'repeat', break and continue
- Better iterators and iterables
- Range literals
- Remove 'Parameter' from visitor

- Match on integers, etc
- Modules
- Characters
- Strings
- Defer/Guard (see Swift)
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
