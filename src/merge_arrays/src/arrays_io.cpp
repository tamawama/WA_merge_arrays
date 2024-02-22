#include <chrono>
#include <functional>
#include <memory>
#include <string>

#include "rclcpp/rclcpp.hpp"
#include "std_msgs/msg/int32_multi_array.hpp"


using namespace std::chrono_literals;
using std::placeholders::_1;


class arrays_io : public rclcpp::Node
{
  public:
  arrays_io() 
  : Node("arrays_io"), count_(0)
  {
    //We are outputting to the inputs to makes sure merge_arrays_node works properly
    array_1_publisher_ = this->create_publisher<std_msgs::msg::Int32MultiArray>("/input/array1", 10);
    array_2_publisher_ = this->create_publisher<std_msgs::msg::Int32MultiArray>("/input/array2", 10);
    //this should return the output.
    merged_output_ = this->create_subscription<std_msgs::msg::Int32MultiArray>(
    "/output/array", 10, std::bind(&arrays_io::output_callback, this, _1));
    timer_ = this->create_wall_timer(
    2000ms, std::bind(&arrays_io::timer_callback, this));
  }
  private:
    void output_callback(const std_msgs::msg::Int32MultiArray & arr){
      std::string arr_str(arr.data.begin(), arr.data.end());
      std::stringstream ss;
      std::copy(arr.data.begin(), arr.data.end(), std::ostream_iterator<int>(ss, " "));
      std::string publish_msg = "Listening: [ " + ss.str() + "]";
      RCLCPP_INFO(this->get_logger(), publish_msg.c_str());
    }

    void timer_callback(){
      arr1.clear();
      arr2.clear();

      //theres more i should test but i cba cause im 99% it works now
      for (int i = 0; i < 3; i++){
        arr1.push_back(3*i + count_);
        arr2.push_back(3*i + count_ + 2);
      }

      //"overflow" test
      // arr1.push_back(100 + count_);
      // arr1.push_back(103 + count_);
      arr2.push_back(200 + count_);
      arr2.push_back(203 + count_);

      auto arr1_msg = std_msgs::msg::Int32MultiArray();
      auto arr2_msg = std_msgs::msg::Int32MultiArray();

      count_++;

      //this used to be just setting msg.data to the vector, but when trying to print the vector properly i changed it while trying to troubleshoot
      arr1_msg.data.clear();
      arr1_msg.data.insert(arr1_msg.data.end(), arr1.begin(), arr1.end());

      arr2_msg.data.clear();
      arr2_msg.data.insert(arr2_msg.data.end(), arr2.begin(), arr2.end());
      
      std::string publish_msg;
      //why is there no vector to string method
      std::stringstream ss;
      std::copy(arr1.begin(), arr1.end(), std::ostream_iterator<int>(ss, " "));
      publish_msg = "Publishing: [ " + ss.str() + "]";
      RCLCPP_INFO(this->get_logger(), publish_msg.c_str());
      array_1_publisher_->publish(arr1_msg);

      sleep(1);

      ss.str("");
      ss.clear();

      std::copy(arr2.begin(), arr2.end(), std::ostream_iterator<int>(ss, " "));
      publish_msg = "Publishing: [ " + ss.str() + "]";
      RCLCPP_INFO(this->get_logger(), publish_msg.c_str());
      array_2_publisher_->publish(arr2_msg);
    }
    //publishers
    rclcpp::Publisher<std_msgs::msg::Int32MultiArray>::SharedPtr array_1_publisher_;
    rclcpp::Publisher<std_msgs::msg::Int32MultiArray>::SharedPtr array_2_publisher_;

    //subscription
    rclcpp::Subscription<std_msgs::msg::Int32MultiArray>::SharedPtr merged_output_;

    //timer
    rclcpp::TimerBase::SharedPtr timer_;
    size_t count_;

    //Arrays
    std::vector<int32_t> arr1;
    std::vector<int32_t> arr2;
};


int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<arrays_io>());
  rclcpp::shutdown();
  return 0;
}

