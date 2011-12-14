
# Watch

  A tiny C program used to periodically execute a command.

## Usage

```

Usage: watch [options] <cmd>

Options:

  -q, --quiet           only output stderr
  -i, --interval <n>    interval in seconds or ms defaulting to 1
  -V, --version         output version number

```

## Installation

```
$ make install
```

Or in your local bin (`~/bin`)

```
$ PREFIX=~ make install
```

## About

  This project is very similar to original [watch(1)](http://linux.die.net/man/1/watch) implemented in 1991, differences include:

  - ansi escape sequences (colors etc)
  - terminal is not cleared
  - lower default interval of 1s
  - millisecond interval resolution

## Milliseconds resolution

 This version of `watch(1)` support millisecond resolution
 with the `ms` suffix:

```
$ watch -i 300ms echo hey
```

whereas `300` would be seconds:

```
$ watch -i 300 echo hey
```

## Examples

 Watch is pretty handy, here are a few use-cases:

### Running tests

  Ad-hoc mtime watchers are annoying to construct,
  and have relatively no purpose when you can simply
  execute your tests at a regular interval. For example
  run `watch(1)` as a job, running tests each second (or a 
  second after the program exits):

```
$ watch make test &
[1] 3794
✔ bifs.components
✔ bifs.dark
✔ bifs.darken
✔ bifs.image-size
...
```

 Your tests will happily chug away, when you want to
 stop watch simply foreground the job and ^C:
 
```
$ fg
```

### Auto-build CSS / JS etc

 Need to build CSS or JavaScript dependencies? use a _Makefile_. With the large quantity of copy-cats (Rake,Jake,Sake,Cake...) people seem to be forgetting that Make is awesome, if you take a little bit of time to learn it you'll love it (or at least most of it). Make will utilize `mtime` and only build what's necessary, this is _great_.

 Let's say we had some Jade templates, even some nested in sub-directories, we could list them in a _Makefile_ quite easily.
 
 Below __JADE__ is a list constructed by the shell command `find templates -name "*.jade"`, which is usually a lot easier to manage than listing these files manually, which is also valid, and sometimes important of ordering is relevant. Following that we have __HTML__ which simply substitutes ".jade" with ".html", giving us our HTML targets. 

```make
JADE = $(shell find templates -name "*.jade")
HTML = $(JADE:.jade=.html)
```

 Our first target is `all`, becoming the default target for `make`. On the right-hand side of this we specify the dependencies, which in this case is a list of all of our HTML files, not yet built. Make will see this and execute the `%.html` targets, which allows use to use the `jade(1)` executable to translate the dependency on the right of `:`, to the target on the left. 

```make
JADE = $(shell find templates -name "*.jade")
HTML = $(JADE:.jade=.html)

all: $(HTML)

%.html: %.jade
	jade < $< > $@
```

 Now we can build all of these files with a single command `make`:

```
$ make
jade < templates/bar.jade > templates/bar.html
jade < templates/baz/raz.jade > templates/baz/raz.html
jade < templates/foo.jade > templates/foo.html
```

 We can also add a `clean` pseudo-target to remove the compiled files with `make clean`. Here it's listed to the right of `.PHONY:`, telling make that it does not expect a file named `./clean` on the fs, so it wont compare mtimes etc. Make is smart about re-executing these actions, if you `make` again you'll notice that since none of the dependencies have changed it'll simply tell you "make: Nothing to be done for `all'.".

```make
JADE = $(shell find templates -name "*.jade")
HTML = $(JADE:.jade=.html)

all: $(HTML)

%.html: %.jade
	jade < $< > $@

clean:
	rm -f $(HTML)

.PHONY: clean
```

  The one missing component is periodical action, which is where `watch(1)` or similar utilities come in, this functionality coupled with Make as a build system creates a powerful duo. 
