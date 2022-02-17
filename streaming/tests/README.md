## How to test streaming code
The tests are an independant part of the code.

The test scope is targeting,
1. Unit testing
2. Component testing - At the moment no mock-up components are going to be used.
3. Loose integration testing -TBD

### Build
cd streaming/tests
mkdir -p build
cmake -S . -B build
cmake --build build

### Run 
./streaming_test_run

### View test results
1. Google Test Framework can extract xml
2. xUnit format can be used to extract to html
3. Goal is an html page view with code coverage

### Add the tests to CI/CD
1. Start with Github actions in a personal repo

### Pros
1. Verify and preserve the current funcitonality
2. Identify build and intergration breaks
3. Increase code coverage
4. Tests are samples of actual code usage that makes "young" developers to understand your code.

### Cons
1. Takes time from development.
1.1 Test 1-2 corner cases that could happen.
1.2 Don't be exhaustive

2. Maintance
2.1 Introduce in the developer culture
2.2 Write the first manual test in code that should be enough
2.3 On issues make tests that cover them. Since you are testing do it once...



### streaming
### replication
### stream compression
