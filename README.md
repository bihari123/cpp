# cpp Notes

## Questions

### Why not use string with 'cin' and long texts

- std::cin has some limitations. It is not practical to get more than a single line using this function. When we need to input larger text among several lines, we use getline().

- **_getline_** is used to input string along several lines fron user input or from files, getline receives the source as its parameters. That can be console or a given file.
