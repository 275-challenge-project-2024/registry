run_tests() {
    while true; do
        echo ""
        echo "Select the test you want to run:"
        echo "1) Run test_client_list"
        echo "2) Run test_worker_heap"
        echo "3) Run both tests"
        echo "4) Exit"
        echo ""
        read -p "Enter your choice (1, 2, 3, or 4): " choice

        case $choice in
            1)
                echo ""
                echo "Running test_client_list..."
                echo "==========================="
                ./test_client_list
                echo "==========================="
                ;;
            2)
                echo ""
                echo "Running test_worker_heap..."
                echo "==========================="
                ./test_worker_heap
                echo "==========================="
                ;;
            3)
                echo ""
                echo "Running both tests..."
                echo "==========================="
                ./test_client_list
                echo "---------------------------"
                ./test_worker_heap
                echo "==========================="
                ;;
            4)
                echo ""
                echo "Exiting."
                echo ""
                exit 0
                ;;
            *)
                echo "Invalid choice. Please select a valid option."
                ;;
        esac
    done
}

# Navigate to the build directory
cd b

# Run the test selection menu
run_tests
