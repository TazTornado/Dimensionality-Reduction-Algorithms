# Software Developement for Algorithmic Problems 
## Project 1 


Σωτηρία Πανταζή 1115201700241
Μηνάς Μάριος Σωτηρίου 1115201700156


## Οργάνωση κώδικα
-----------------------------------
Στο παραδοτέο αρχείο .tar θα βρείτε τον αρχικό φάκελο του project, Project1, του οποίου τα περιεχόμενα έχουν την ακόλουθη δομή:


Project1   
>			-> src (dir)	-> main_lsh.cpp
				  -> main_cube.cpp
				  -> main_cluster.cpp
				  -> lsh.cpp
				  -> hypercube.cpp
				  -> kmeans.cpp
				  -> utilities.cpp
				  -> hashtable.cpp
				  -> tree.cpp
				  -> item.cpp

>			-> inc (dir)    -> lsh.h
				  -> hypercube.h
				  -> kmeans.h
				  -> utilities.hpp
				  -> hashtable.hpp
				  -> tree.hpp
				  -> item.hpp
				  -> error.h
 >           -> Makefile

Κατά τη μεταγλώττιση δημιουργούνται οι φάκελοι obj και bin, στους οποίους θα μπουν τα αντικειμενικά αρχεία και τα εκτελέσιμα αντίστοιχα.


## Μεταγλώττιση & Εκτέλεση Κώδικα
-----------------------------------
Στον φάκελο Project1, για να μεταγλωττίσετε τον κώδικα, τρέχετε ένα από τα εξής:

> \$ make (all)		// και τα δύο μέρη και τα dependencies τους
> \$ make A			// Μέρος Α: lsh, hypercube και τα dependencies τους
> \$ make lsh		// lsh και deps
> \$ make cube		// hypercube και deps
> \$ make cluster	// Μέρος Β: kmeans και  dependencies


και για να διαγράψετε τα αντικειμενικά και εκτελέσιμα αρχεία

> $ make clean		// ΔΕΝ διαγράφει τα αρχεία dataset, configuration, ή output


Για να εκτελέσετε κάποιο από τα προγράμματα, τρέχετε κάποιο από τα εξής:

> $ **./lsh –i \<input file> –q \<query file> –k \<int> -L \<int> -ο \<output file> -Ν \<number of nearest> -R \<radius>** *// Οι default τιμές L, k με w == 600 δίνουν πολύ καλή ακρίβεια και ταχύτητα στον αλγόριθμο*

> $ **./cube –i \<input file> –q \<query file> –k \<int> -M \<int> -probes \<int> -ο \<output file> -Ν \<number of nearest> -R \<radius>**  *// Με κ = 14 , probes = 56 , m == 1000 πετυχαίνουμε μία αρκετά καλή ακρίβεια. Ο χρόνος είναι πολύ καλός σε κάθε περίπτωση*



> $ **./cluster –i \<input file> –c \<configuration file> -o \<output file> -complete \<optional> -m \<Classic ή LSH ή Hypercube>** *// Ο αλγόριθμος lloyd's είναι πολύ γρήγορος και δίνει total Silhouette περίπου 0.15 για τιμές K-medians = 3, 5, 6*

#### Σημείωση:
Εάν το path του αρχείου output_file περιέχει φακέλους που δεν έχουν δημιουργηθεί, θα πρέπει να δημιουργηθούν πριν την εκτέλεση του προγράμματος.


### Link του project στο GitLab:
https://gitlab.com/MinasMar/project-project1.git


### Μοντελοποίηση & Υλοποίηση
-----------------------------------

#### Η δομή item
Τα πολυδιάστατα σημεία του input set υλοποιούνται ως μια κλάση **item**, η οποία περιέχει τις λεπτομέρειες ενός σημείου και υποστηρίζει τις εξής βοηθητικές συναρτήσεις:
>`change_certain_dimension()`
`get_certain_dimenstion()`
`get_length()`
`get_elements()`
`get_id()`
`item_distance()`
`item_distance_squared()`
`printitem()` // overloaded`


#### Διπλή συμπεριφορά του hashtable για το LSH και το Hypercube
Στη περίπτωση του lsh λειτουργεί σαν κανονικό hashtable με buckets, που είναι δυαδικά δέντρα ταξινομημένα με βάση τα ID (querying trick). Από την άλλη στο hypercube ξέρουμε ότι είναι πιθανό να μη γεμίσουν όλα τα buckets έτσι έχουμε μια υλοποίηση map της stl της c++, με κλειδιά τα labels των buckets σε μορφή string, που αντιστοιχίζονται σε vectors από τα items.
Υπάρχουν διάφορες συναρτήσεις που μας βοηθούν κιόλας και στη υλοποίηση του reverse assignment στο clustering. Σε αυτά τα αρχεία εμπεριέχονται οι πιο σημαντικές και οι περισσότερες διαδικασίες 


#### Η δομή του δέντρου
Είναι μια κλασσική υλοποίηση δέντρου με "key" τα ids και nodes μία λίστα από τα στοιχεία με ίδιο ID.


#### Το αντικείμενο LSH και Hypercube
Ο αλγόριθμος LSH υλοποιείται με την κλάση **LSH**, η οποία με την αρχικοποίησή της αποθηκεύει τις παραμέτρους της. Έχει τις βασικές λειτουργίες `preprocess_item()` και `execute_query()`, που καλούν τις απαραίτητες συναρτήσεις στην κλάση hashtable για να εισάγουν τα στοιχεία στο database και κάνουν την εξαντλητική αναζήτηση αντίστοιχα.

Με ακριβώς αντίστοιχη λογική υλοποιείται και η τυχαία προβολή στον υπερκύβο, με την κλάση **Hypercube**.



#### Tο Clustering
Περιέχουν τα πάντα για το clustering, εκεί υλοποιείται η lloyd υλοποίηση, τα reverse assignment lsh και Hypercube.
Πέρα από ειδικές για συναρτήσεις για κάθε υλοποίηση που ζητείται, υπάρχουν γενικές συναρτήσεις που βοηθούν στην υλοποίηση και των 3 ειδών clustering (`update_centroid`, `calculate_median_distance`)

Τα main_* αρχεία είναι απλές διεπαφές για την υλοποίηση των ζητούμενων


Τα utilities αρχεία περιέχουν διάφορες χρήσιμες global μεταβλητές για την εύρεση στατιστικών και διάφορες χρήσιμες μεθόδους για κάθε ανάγκη σε κάθε διαδικασία, για αυτό εμπεριέχεται το αρχείο utilities.hpp σε κάθε αρχείο.

Συναρτήσεις που εμπεριέχονται επιγραμματικά:

> `error_handler()`
`euclidean_modulo()`
`gaussian_random_generator()`
`uniform_random_generator()` *// overloaded*
`read_configuration()` 
`get_program_parameters()`
`count_lines_in_file()`
`parse_file()`
