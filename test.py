class Animal:
    def __init__(self, name):
        self.name = name
    
    def make_sound(self):
        pass

class Dog(Animal):
    def __init__(self, name, breed):
        super().__init__(name)
        self.breed = breed
    
    def make_sound(self):
        return "Woof!"

def process_list(numbers):
    # List comprehension example
    squares = [x * x for x in numbers]
    return squares

def process_dict(numbers):
    # Dictionary example
    number_map = {str(n): n for n in numbers}
    return number_map

def process_numbers(numbers):
    try:
        # Process the list
        result = process_list(numbers)
        
        # Process the dictionary
        number_map = process_dict(numbers)
        
        # Calculate sum using range
        total = 0
        for i in range(len(numbers)):
            total += numbers[i]
        
        # Check for negative numbers
        for num in numbers:
            if num < 0:
                raise ValueError("Negative number found")
        
        return result, number_map, total
    
    except ValueError as e:
        print(f"Error: {e}")
        return [], {}, 0
    except Exception as e:
        print(f"Unexpected error: {e}")
        return [], {}, 0

# Test the functionality
if __name__ == "__main__":
    # Create and test a Dog object
    dog = Dog("Rex", "German Shepherd")
    print(f"Dog says: {dog.make_sound()}")
    
    # Test number processing
    numbers = [1, 2, 3, 4, 5]
    results = process_numbers(numbers)
    print(f"Results for {numbers}: {results}")
    
    # Test error handling
    negative_numbers = [1, -2, 3]
    error_results = process_numbers(negative_numbers)
    print(f"Results for {negative_numbers}: {error_results}") 