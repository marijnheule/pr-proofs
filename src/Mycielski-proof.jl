function graph(m)
    global nv, edges, cross, w_edges
    if m < 2
        println("tried to generate graph smaller than 2")
    elseif m == 2
        push!(edges[2], (1, 2))
        nv[2] = 2
    else
        graph(m - 1)
        es = edges[m - 1]
        v = nv[m - 1]
        edges[m] = deepcopy(es)
        for i = 1:v
            for e in es
                if e[1] == i
                    push!(edges[m], (i, e[2] + v))
                    push!(cross[m], (i, e[2] + v))
                end
                if e[2] == i
                    push!(edges[m], (e[1] + v, i))
                    push!(cross[m], (e[1] + v, i))
                end
            end
        end
        w = 2v + 1
        for i = (v + 1):2v
            push!(edges[m], (i, w))
        end
        for e in w_edges[m - 1]
            push!(w_edges[m], (e[1] + v, e[2] + v))
        end
        for i = (v + 1):2v
            push!(w_edges[m], (i, w))
        end
        nv[m] = w
    end
end

function index(i, c)
    global nc
    return (i-1) * nc + c
end

function addblocked(m)
    for i in 1:nv[m]
        for c in 1:nc
            for d in (c + 1):nc
                println("-$(index(i, c)) -$(index(i, d)) 0")
            end
        end
    end
end

function addpr(m)
    global equiv
    v = nv[m - 1]
    s = 0
    for i = m:(k - 1)
        s += nv[i]
    end
    w = nv[m] + s

    for i = (s + 1):(s + v)
        for c = 1:nc
            for d = 1:nc
                if c == d
                    continue
                end
                witness = [-index(v + i, d), index(v + i, c), index(i, c), -index(w, c)]
                for h in findnodes(v + i)
                    append!(witness, (-index(h, d), index(h, c)))
                end
                println("-$(index(v + i, d)) -$(index(i, c)) $(index(w, c)) " * join(witness, " ") * " 0")
            end
        end
    end

    for i = (s + 1):(s + v)
        for c = 1:nc
            for d = 1:nc
                if c == d
                    continue
                end
                witness = [-(index(i, d)), (index(v + i, c)), (index(i, c))]
                for h in findnodes(i)
                    append!(witness, (-index(h, d), index(h, c)))
                end
                println("-$(index(i, d)) -$(index(v + i, c)) " * join(witness, " ") * " 0")
            end
        end
        if !(v + i in keys(equiv))
            equiv[v + i] = []
        end
        push!(equiv[v + i], i)
    end
end

function findnodes(i)
    global equiv
    if !(i in keys(equiv))
        return []
    else
        nodes = []
        for j in equiv[i]
            push!(nodes, j)
            append!(nodes, findnodes(j))
        end
        return nodes
    end
end

function addclique(m)
    clique = collect((nv[m] - m + 1):(nv[m]))

    for i = 1:(m - 1)
        cc = nc - i + 1
        l = clique[end - i + 1]
        for x in clique[1:end - i]
            for c in 1:(cc - 1)
                witness = [-(index(x, cc)), -(index(l, c)), (index(x, c)), (index(l, cc))]
                for h in findnodes(x)
                    append!(witness, (-index(h, cc), index(h, c)))
                end
                for h in findnodes(l)
                    append!(witness, (-index(h, c), index(h, cc)))
                end
                println("-$(index(x, cc)) -$(index(l, c)) " * join(witness, " ") * " 0")
            end
            println("-$(index(x, cc)) 0")
        end
    end
end

function proof(m)
    global k
    if m < 2
        println("proof < 2")
    elseif m == 2
        addclique(k)
    else
        addpr(m)

        proof(m - 1)
    end
end

function main(args)
    global k, nc, nv, edges, cross, w_edges, equiv

    k = parse(Int, args[1])
    nc = parse(Int, args[2])

    nv = zeros(Int, k)

    edges = [[] for _ = 1:k]
    cross = [[] for _ = 1:k]
    w_edges = [[] for _ = 1:k]
    equiv = Dict()

    graph(k)
    addblocked(k)
    proof(k)
end

main(ARGS)
