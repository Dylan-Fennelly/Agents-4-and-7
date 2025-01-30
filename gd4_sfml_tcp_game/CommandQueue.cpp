/*Albert Skalinski - D00248346
  Dylan Fennelly - D00248176*/

#include "CommandQueue.hpp"

void CommandQueue::Push(const Command& command)
{
    m_queue.push(command);
}

Command CommandQueue::Pop()
{
    Command command = m_queue.front();
    m_queue.pop();
    return command;
}

bool CommandQueue::IsEmpty() const
{
    return m_queue.empty();
}
