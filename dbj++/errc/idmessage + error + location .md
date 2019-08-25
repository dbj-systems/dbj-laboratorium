# The Design
### The Most Usefull Type
- The MUT?
```
// pseudo code
STRUCTURE IDMESSAGE_TYPE {
   ID_TYPE			ID ;
   MESSAGE_TYPE		MESSAGE;
}

```
- { ID, MESSAGE } is the "base of everything" in its utility
- A.k.a.: "the most usefull data type"

```
STRUCTURE ERROR_AND_LOCATION {
      IDMESSAGE_TYPE ERROR ;
	  OPTIONAL MESSAGE_TYPE LOCATION ;
}
// no unnecessary typedef's above

```
