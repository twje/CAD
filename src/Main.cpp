#include <SFML/Graphics.hpp>

#include <iostream>

class Shape;

void DrawPoint(sf::RenderTarget& target, sf::Vector2f position, float radius, sf::Color color)
{
    sf::CircleShape shape(radius);
    shape.setFillColor(color);
    shape.setPosition({ position.x - radius, position.y - radius });
    target.draw(shape);
}

class Node
{
public:
    Node(Shape* parent, const sf::Vector2f& position)
        : mParent(parent)
        , mPosition(position)
    { }

    void SetPosition(const sf::Vector2f& position)
    {
        mPosition = position;
    }

    const sf::Vector2f& GetPosition() const { return mPosition; }

public:
    Shape* mParent;
    sf::Vector2f mPosition;
};

class Shape
{
public:    
    virtual ~Shape() = default;

    Shape(size_t maxNodes)
        : mMaxNodes(maxNodes)
        , mColor(sf::Color::Green)
    {
        mNodes.reserve(maxNodes);
    }    

    Node* GetNextNode(const sf::Vector2f& position)
    {
        if (mNodes.size() == mMaxNodes)
        {
            return nullptr;
        }

        Node node(this, position);
        mNodes.push_back(node);
        return &mNodes[mNodes.size() - 1];
    }

    const Node& GetNode(size_t index) const { return mNodes[index]; }
    const sf::Color& GetColor() { return mColor; }

    void SetColor(const sf::Color& color) { mColor = color; }

    void DrawNodes(sf::RenderTarget& target)
    {
        for (const Node& node : mNodes)
        {
            DrawPoint(target, node.GetPosition(), 3.0f, sf::Color::Red);
        }
    }

    virtual void DrawShape(sf::RenderTarget& target) = 0;

private:
    std::vector<Node> mNodes;
    size_t mMaxNodes;
    sf::Color mColor;
};

class Line : public Shape
{
public:
    Line()
        : Shape(2)
    { }

    virtual void DrawShape(sf::RenderTarget& target)
    {
        sf::Vertex line[] = {
            GetNode(0).GetPosition(),
            GetNode(1).GetPosition()
        };

        line[0].color = GetColor();
        line[1].color = GetColor();

        target.draw(line, 2, sf::PrimitiveType::Lines);
    }
};

enum class ShapeType
{
    NONE = 0,
    LINE = 1
};

class Application
{
public:
    Application()
        : mWindow(sf::VideoMode(sf::Vector2u(1600, 960), 32), "SFML works!")
    {
        mView = mWindow.getDefaultView();
    }

    void Run()
    {
        bool isMiddleButtonPressed = false;        
        sf::Vector2i lastPanPosition;        
        
        Shape* tempShape = nullptr;
        Node* selectedNode = nullptr;
        ShapeType createShape = ShapeType::NONE;

        while (mWindow.isOpen())
        {
            sf::Vector2i mousePosition = sf::Mouse::getPosition(mWindow);
            float zoomIncrement = 0.0f;
            bool isLeftButtonJustReleased = false;

            sf::Event event;
            while (mWindow.pollEvent(event))
            {
                if (event.type == sf::Event::Closed)
                {
                    mWindow.close();
                }

                // Pan
                if (event.type == sf::Event::MouseButtonPressed)
                {
                    if (event.mouseButton.button == sf::Mouse::Button::Middle)
                    {
                        lastPanPosition = mousePosition;
                        isMiddleButtonPressed = true;                        
                    }                                       
                }

                if (event.type == sf::Event::MouseButtonReleased)
                {
                    if (event.mouseButton.button == sf::Mouse::Button::Middle)
                    {
                        isMiddleButtonPressed = false;
                    }

                    if (event.mouseButton.button == sf::Mouse::Button::Left)
                    {
                        isLeftButtonJustReleased = true;
                    }
                }

                // Zoom
                if (event.type == sf::Event::MouseWheelScrolled)
                {
                    if (event.mouseWheelScroll.wheel == sf::Mouse::Wheel::Vertical)
                    {                                   
                        if (event.mouseWheelScroll.delta == 1)
                        {
                            zoomIncrement = mZoomSpeed;
                        }
                        else if (event.mouseWheelScroll.delta == -1)
                        {
                            zoomIncrement = -mZoomSpeed;
                        }   
                    }
                }

                // Shapes
                if (sf::Keyboard::isKeyPressed(sf::Keyboard::Key::L))
                {
                    createShape = ShapeType::LINE;
                }
            }            

            if (zoomIncrement != 0)
            {
                sf::Vector2f mouseWorldBeforeZoom = mWindow.mapPixelToCoords(mousePosition, mView);
                mView.zoom(1.0f / mZoomFactor);  // Reset zoom factor to 1.0f
                mZoomFactor = Clamp(mZoomFactor + zoomIncrement, mZoomMin, mZoomMax);
                mView.zoom(mZoomFactor);

                sf::Vector2f mouseWorldAfterZoom = mWindow.mapPixelToCoords(mousePosition, mView);
                sf::Vector2f adjustment = mouseWorldBeforeZoom - mouseWorldAfterZoom;
                mView.move(adjustment);
            }

            if (isMiddleButtonPressed)
            {          
                // Convert the last pan position and current mouse position to world coordinates
                sf::Vector2f worldLastPanPosition = mWindow.mapPixelToCoords(lastPanPosition, mView);
                sf::Vector2f worldMousePosition = mWindow.mapPixelToCoords(mousePosition, mView);
                
                // Calculate the pan offset in world coordinates
                sf::Vector2f panOffsetWorld = worldLastPanPosition - worldMousePosition;
                mView.move(panOffsetWorld);

                // Update the last pan position (in screen coordinates)
                lastPanPosition = mousePosition;               
            }

            sf::Vector2f worldMousePosition = mWindow.mapPixelToCoords(mousePosition, mView);

            float nearestGridX = std::round(worldMousePosition.x / mGridSpacing) * mGridSpacing;
            float nearestGridY = std::round(worldMousePosition.y / mGridSpacing) * mGridSpacing;

            mCursor.x = nearestGridX;
            mCursor.y = nearestGridY;

            if (createShape != ShapeType::NONE)
            {
                switch (createShape)
                {
                    case ShapeType::LINE:
                    {
                        tempShape = new Line();
                        selectedNode = tempShape->GetNextNode(mCursor);
                        selectedNode = tempShape->GetNextNode(mCursor);
                        break;
                    }
                }
                createShape = ShapeType::NONE;
            }

            if (selectedNode != nullptr)
            {
                selectedNode->SetPosition(mCursor);
            }

            if (isLeftButtonJustReleased)
            {
                if (tempShape != nullptr)
                {
                    selectedNode = tempShape->GetNextNode(mCursor);
                    if (selectedNode == nullptr)
                    {
                        tempShape->SetColor(sf::Color::White);
                        mShapes.push_back(tempShape);
                        tempShape = nullptr;
                    }
                }
                else
                {
                    selectedNode = nullptr;
                }
            }

            mWindow.setView(mView);
            mWindow.clear();
            
            sf::Vector2f viewSize = mView.getSize();
            sf::Vector2f topLeftWorld = mWindow.mapPixelToCoords({ 0, 0 }, mView);

            topLeftWorld.x = std::floor(topLeftWorld.x / mGridSpacing) * mGridSpacing;
            topLeftWorld.y = std::floor(topLeftWorld.y / mGridSpacing) * mGridSpacing;

            for (size_t gridX = 0; gridX < viewSize.x + mGridSpacing; gridX += mGridSpacing)
            {
                for (size_t gridY = 0; gridY < viewSize.y + mGridSpacing; gridY += mGridSpacing)
                {
                    DrawPoint(mWindow, { gridX + topLeftWorld.x, gridY + topLeftWorld.y }, 3.0f, sf::Color::Green);
                }
            }
            
            for (Shape* shape : mShapes)
            {
                shape->DrawShape(mWindow);
                shape->DrawNodes(mWindow);
            }

            if (tempShape != nullptr)
            {
                tempShape->DrawShape(mWindow);
                tempShape->DrawNodes(mWindow);
            }

            DrawPoint(mWindow, mCursor, 3.0f, sf::Color::Yellow);
            
            mWindow.display();           
        }
    }

private:
    float Clamp(float value, float minValue, float maxValue) 
    {
        return std::max(minValue, std::min(value, maxValue));
    }

    sf::RenderWindow mWindow;    
    sf::View mView;    
    float mZoomFactor = 1.0f;
    float mZoomSpeed = 0.1f;
    float mZoomMin = 0.3f;
    float mZoomMax = 1.7f;
    float mGridSpacing = 70.0f;
    sf::Vector2f mCursor;
    std::vector<Shape*> mShapes;
};

int main()
{
    Application app;
    app.Run();

    return 0;
}