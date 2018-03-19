# Notes on maintaining compatibility with Stata 12

## A) Mata functions added in Stata 13 and their pre-13 substitutes

### 1. selectindex(_v_) <-> select(_X, v_)

#### Description

Considering the case of vector inputs, while select(_X, v_) returns the values of X for which v!=0, selectindex(_v_) returns the vector of _indexes_. So, to emulate **selectindex** with **select** one has to emulate X as an index vector, which _would_ mean

	select(1..length(v),v)

for a row vector, or

	select(1::length(v),v)

for a column vector.

**However**, because select(_<scalar>_,0) returns a 0x0 matrix nonconformable with scalars for concatenation into a vector, there needs to be a workaround forcing select to always return vectors:

	select(1..length(v)+1,(v,0))

for a row vector, or

	select(1::length(v)+1,(v\0))

for a column vector.


#### Affected procedures

	- gcollapse
	- gcontract
	- gtoplevelsof
	- \_gtools\_internal
