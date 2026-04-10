#include <iostream>
#include <vector>
#include <string>


class Resource{
public:
    virtual ~Resource() = default;
    virtual std::string getDescription() const = 0;
};

//custom file deleter for FILE*

struct FileDeleter {
    std::string fname;
    void operator()(FILE* fp) const{
        if(fp){
            std::cout<<"Closing File: "<<fname<<std::endl;
            fclose(fp);
        }
    }
};

class FileResource : public Resource{
private:
    std::unique_ptr<FILE, FileDeleter> filePtr_;
    std::string fname_;
public:
    explicit FileResource(const std::string& fname) : fname_(fname){
        std::cout<<"creating file resource .... "<<fname<<std::endl;
        FILE* fp = fopen(fname.c_str(), "w");
        if(!fp){
            throw std::runtime_error("Failed to open file: " + fname);
        }
        filePtr_= std::unique_ptr<FILE, FileDeleter>(fp, FileDeleter{fname});
    }

    std::string getDescription() const override{
        return "File: " + fname_;
    }
};

class ResourceManager {
    private:
        std::vector<std::unique_ptr<Resource>> resourcesList_;

    public:
        //add
        void addResource(std::unique_ptr<Resource> res){
            resourcesList_.push_back(std::move(res));
        }

        //remove at specified index
        std::unique_ptr<Resource> removeResource(size_t index){
            if (index >= resourcesList_.size()){
                throw std::out_of_range("Invalid resource index");
            }
            auto resource = std::move(resourcesList_[index]);
            resourcesList_.erase(resourcesList_.begin() + index);
            return resource;
        }

        //resource count
        size_t getResourceCount() const {
            return resourcesList_.size();
        }

        //List all active resources 
        void listResources(){
            std::cout<<" Listing Resources - "<<std::endl;
            for(const auto& res : resourcesList_){
                std::cout<<" - "<<res->getDescription() <<std::endl;
            }
        }

        // Destructor automatically cleans up all resources
        ~ResourceManager() {
            std::cout << "Resources cleaned up" << std::endl;
        }
};

int main(){
    ResourceManager rm;

    // Add a few file resources
    rm.addResource(std::make_unique<FileResource>("test1.txt"));
    rm.addResource(std::make_unique<FileResource>("test2.txt"));
    rm.addResource(std::make_unique<FileResource>("test3.txt"));

    // List all resources
    rm.listResources();

    std::cout << "Removing first resource..." << std::endl;

    // Remove the first resource
    rm.removeResource(0);

    // List resources again
    rm.listResources();

    return 0;
}
