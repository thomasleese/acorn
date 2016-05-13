# Typing

The type system of Acorn is static, strong and built around a powerful inference engine. This means that the types of variables and functions are checked during compile time to make sure they are all compatible and also that types must follow strict rules to be classed as compatible. Although this is different to dynamic languages like Python or weak languages like PHP or C, you will hardly notice a difference because the inference engine can do a lot of work for you.




    let Integer as Type{Integer32} = Integer32
    
    type Pair{T, U} as Tuple{T, U}
    type List{T} as Sequence{T}
    
    def make_list{T}(element as T) as List{T}
        [element]
    end
