extern mod extra;
use time_ns = extra::time::precise_time_ns;

pub struct Grid {
    // maximum grid size 16x16. switch to vector if size gets unweildy, switch away from u8 if
    // larger grid needed.
    priv cells: [u8, ..256],
    priv start: (uint, uint),
    priv end: (uint, uint),
    priv n: uint
}

fn at(n: uint, x: uint, y: uint) -> uint {
    assert!(x < n);
    assert!(y < n);
    (n * y) + x
}

impl Grid {
    fn at(&self, x: uint, y: uint) -> uint {
        at(self.n, x, y)
    }
    fn ta(&self, idx: uint) -> (uint, uint) {
        (idx % self.n, idx / self.n)
    }

    pub fn get_n(&self) -> uint {
        self.n
    }

    pub fn display(&self) {
        for y in range(0, self.n) {
            for x in range(0, self.n) {
                print!("{: >3}", self.cells[self.at(x, y)]);
            }
            print!("\n");
        }
    }

    fn empty(n: uint) -> Grid {
        Grid { cells: [0, ..256], start: (0, 0), end: (0, 0), n: n }
    }

    pub fn new(n: uint, border: &[u8]) -> Grid {
        use std::hashmap::HashSet;

        if n > 16 {
            fail!("Squares larger than 16 not supported");
        }

        let expected_cells = n*2 + (n-2)*2;
        if border.len() != expected_cells {
            fail!("Expected {} cells, found {}", expected_cells, border.len());
        }

        let h: HashSet<u8> = border.iter().map(|x| *x).collect();
        if h.len() != border.len() {
            fail!("Duplicates in border");
        }

        let mut g = Grid::empty(n);

        {
            let mut it = border.iter();
            let grid = &mut g.cells;

            // optimization: these 4 can be done in parallel, though there will be cacheline contention
            // which may make it a misopt.
            for i in range(0, n) {
                // optimization: avoid bounds checks
                grid[at(n, i, 0)] = *it.next().unwrap();
            }

            for i in range(1, n) {
                grid[at(n, n-1, i)] = *it.next().unwrap();
            }

            for i in range(1, n) {
                grid[at(n, n-i-1, n-1)] = *it.next().unwrap();
            }

            for i in range(2, n) {
                grid[at(n, 0, n-i)] = *it.next().unwrap();
            }
        }

        g.start = g.ta(g.cells.iter().enumerate().filter(|&(_, v)| *v != 0)
                       .min_by(|&(_, v)| v).unwrap().n0());
        g.end = g.ta(g.cells.iter().enumerate().filter(|&(_, v)| *v != 0)
                     .max_by(|&(_, v)| v).unwrap().n0());

        g
    }

    /// Attempt to solve the grid in-place. Returns true if solving succeeded.
    pub fn solve(&mut self) -> bool {
        if self.cells.iter().skip(self.n + 1).any(|x| *x == 0) {
            // check if all the numbers are set already (only in 1x1 and 2x2)
            self.solve_inner(self.start, 1)
        } else {
            true
        }
    }

    fn solve_inner(&mut self, (x,  y): (uint, uint), value: u8) -> bool {
        debug!("Solving at ({}, {}) with value {} and {:?}", x, y, value, self.display());

        let m = (self.n * self.n) as u8;
        if value > m {
            fail!("Somehow we have recursed deeper than the number of cells!");
        } else if value == m {
            return true; // yay we win
        }

        let neighs = self.neighbors((x, y));
        match neighs.iter().find(|& &(_, _, v)| v == value+1) {
            // cell is already next to us.
            Some(&(x, y, _)) => self.solve_inner((x, y), value+1),
            None => {
                for &(x, y, _) in neighs.iter().filter(|& &(_, _, v)| v == 0) {
                    self.cells[self.at(x, y)] = value+1;
                    if self.solve_inner((x, y), value+1) {
                        return true;
                    } else {
                        self.cells[self.at(x, y)] = 0;
                    }
                }
                return false; // we lose :(
            }
        }
    }

    fn neighbors(&self, (x, y): (uint, uint)) -> ~[(uint, uint, u8)] {
        let check = [(x-1, y), (x+1, y), (x, y-1), (x, y+1)];
        let mut vs = std::vec::with_capacity(4);
        for &(x, y) in check.iter() {
            if x < self.n && y < self.n {
                vs.push((x, y, self.cells[self.at(x, y)]));
            }
        }
        vs
    }
}

impl FromStr for Grid {
    fn from_str(s: &str) -> Option<Grid> {
        let ints = s.split(' ').map(|x| from_str::<u8>(x.trim()).unwrap()).to_owned_vec();
        Some(Grid::new(ints[0] as uint, ints.slice_from(1)))
    }
}

fn main() {
    use std::io::stdio::stdin;
    use std::io::buffered::BufferedReader;
    let mut sin = BufferedReader::new(stdin());
    let args = std::os::args();

    match args.get_opt(1) {
        Some(&~"display") => {
            let g: Grid = from_str(sin.read_line().unwrap()).unwrap();
            g.display();
        },
        Some(&~"ddisplay") => {
            let g: Grid = from_str(sin.read_line().unwrap()).unwrap();
            g.display();
            println!("Start: {:?}, End: {:?}", g.start, g.end);
        },
        Some(&~"ta-test") => {
            let l = sin.read_line().unwrap();
            let n = from_str::<uint>(l.trim()).unwrap();
            let g = Grid::empty(n);
            for i in range(0, n*n) {
                let (x, y) = g.ta(i);
                println!("({}, {})", x, y);
            }
        },
        Some(_) => fail!("Unrecognized command"),
        None => {
            println("Solving grids; enter grid specifiers on own line");
            for line in sin.lines() {
                let start = time_ns();
                let mut g: Grid = from_str(line).expect("invalid grid");
                let end = time_ns();

                if g.solve() {
                    println!("Solved a {0}x{0} in {1} ns", g.get_n(), end - start);
                    g.display();
                } else {
                    println("No alibi");
                }
            }
        }
    }
}
