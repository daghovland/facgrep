# facgrep
<p><a href="http://www.ii.uib.no/~dagh">Dag Hovland</a></p>
<h1>Search using regular expressions with numerical constraints</h1>
<p>The article <a href="http://hdl.handle.net/1956/3628">Regular Expressions with Numerical Constraints and Automata with Counters</a> (ICTAC 2009) describes a new algorithm for searching for strings matching regular epxressions with numerical constraints in text documents. The algorithm is implemented in a manner similar to Gnu egrep (<a href="http://www.gnu.org/software/grep">http://www.gnu.org/software/grep</a>). It lacks features to be a full replacement for grep.</p> 

<p>The algorithm is, unlike the one used in GNU grep, polynomial time, but only <i>extended regular expressions</i> that are <i>counter-1-unmabiguous</i> are allowed. The program exits with a warning if given other epxressions. With the flag "-g" it outputs  information about the "Finite Automaton with Counter" (FAC) which is constructed by the program, and how the FAC matches the string.</p>

<h2>Building</h2>
<p>Run "make". You need at least flex, bison, bash and gcc.</p> 

<h2>Running</h2>
<p>The executable is "grep". The available flags are "-x" for exact matches, also called "line-regexp" (usually it returns a match if some part of a line matches) "-g" for printing out the full FAC and matching process, "-v" for inverted match, "-n" for printing line numbers of occurrences, "-E" for compatibility with grep (extended regular expressions are the standard) and "-c" for counting number of matches. Note that the regular expressions supported are those usually called  "extended regular expressions".</p>

<h2>Download</h2>
<p><a href="facgrep.tar.bz2">facgrep.tar.bz2</a></p>
<p><i>Last updated: 11. September 2009</i></p>
<p>
    <a href="http://validator.w3.org/check?uri=referer"><img
        src="http://www.w3.org/Icons/valid-xhtml10-blue"
        alt="Valid XHTML 1.0 Strict" height="31" width="88" /></a>
  </p>

