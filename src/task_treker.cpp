#include <iostream>
#include <tuple>
#include <map>
using namespace std;

// Перечислимый тип для статуса задачи
enum class TaskStatus {
  NEW,          // новая
  IN_PROGRESS,  // в разработке
  TESTING,      // на тестировании
  DONE          // завершена
};

// Объявляем тип-синоним для map<TaskStatus, int>,
// позволяющего хранить количество задач каждого статуса
using TasksInfo = map<TaskStatus, int>;

class TeamTasks {
public:
  // Получить статистику по статусам задач конкретного разработчика
  const TasksInfo& GetPersonTasksInfo(const string& person) const {
	  return developers_base.at(person);
  }

  // Добавить новую задачу (в статусе NEW) для конкретного разработчитка
  void AddNewTask(const string& person) {
	  ++developers_base[person][TaskStatus::NEW];
  }

  // Обновить статусы по данному количеству задач конкретного разработчика,
  // подробности см. ниже
  tuple<TasksInfo, TasksInfo> PerformPersonTasks(
      const string& person, int task_count) {
	  auto tasks_info_tmp = developers_base[person];
	  TasksInfo new_tasks;
	  TasksInfo old_tasks;

	  RemoveTasks(TaskStatus::NEW, TaskStatus::IN_PROGRESS, task_count, tasks_info_tmp, new_tasks, old_tasks);
	  RemoveTasks(TaskStatus::IN_PROGRESS, TaskStatus::TESTING, task_count, tasks_info_tmp, new_tasks, old_tasks);
	  RemoveTasks(TaskStatus::TESTING, TaskStatus::DONE, task_count, tasks_info_tmp, new_tasks, old_tasks);


	  TasksInfo persons_tasks;
	  for(const auto& it : new_tasks) {
		 /* if (persons_tasks.count(it.first) == 0) {
			  persons_tasks[it.first] = it.second;
		  } else {*/
			  persons_tasks[it.first] += it.second;
		 // }
	  }
	  for(const auto& it : old_tasks) {
		  /*if (persons_tasks.count(it.first) == 0) {
			  persons_tasks[it.first] = it.second;
		  } else {*/
			  persons_tasks[it.first] += it.second;
		  //}
	  }
	  //cout << persons_tasks[TaskStatus::NEW] << endl;
	  //cout << new_tasks.size() << endl;
	  developers_base[person] = persons_tasks;

	  return make_tuple(new_tasks, old_tasks);
  }
private:
  void RemoveTasks(TaskStatus from, TaskStatus to, int& task_cnt, TasksInfo& prsn,
		  	  	  	  	  	  	  	  	  	  	  	  	  TasksInfo& new_t, TasksInfo& old_t) {
	  if (prsn[from] != 0 && task_cnt > 0) {
	  		  int tasks_new_temp = prsn[from];
	  		  int tasks_inprogress_temp = prsn[to];

	  		  if (task_cnt <= tasks_new_temp) {
	  			new_t[to] += task_cnt;
	  			tasks_new_temp-= task_cnt;
	  			task_cnt -= task_cnt;
	  			cout << "111" << endl;
	  		  } else {
	  			  new_t[to] += tasks_new_temp;
	  			  task_cnt -= tasks_new_temp;
	  			cout << "222" << endl;
	  		  }
	  		  if (tasks_new_temp != 0) {
	  			  old_t[from] += tasks_new_temp;
	  			cout << "333" << endl;
	  		  }
	  		  if (tasks_inprogress_temp != 0) {
	  			old_t[to] += tasks_inprogress_temp;
	  			cout << "444" << endl;
	  		 }
	  	  }
  }

  map<string, TasksInfo> developers_base;
};


/*
#include <algorithm>
#include <map>
#include <string>
#include <tuple>
#include <vector>

using namespace std;

// Выделим в отдельную функцию получение следующего по приоритету типа задачи
// Функция налагает требование на входной параметр: он не должен быть равен DONE
// При этом task_status явно не сравнивается с DONE, что позволяет
// сделать эту функцию максимально эффективной
TaskStatus Next(TaskStatus task_status) {
  return static_cast<TaskStatus>(static_cast<int>(task_status) + 1);
}

// Объявляем тип-синоним для map<TaskStatus, int>,
// позволяющего хранить количество задач каждого статуса
using TasksInfo = map<TaskStatus, int>;

class TeamTasks {
public:
  // Получить статистику по статусам задач конкретного разработчика
  const TasksInfo& GetPersonTasksInfo(const string& person) const;

  // Добавить новую задачу (в статусе NEW) для конкретного разработчика
  void AddNewTask(const string& person);

  // Обновить статусы по данному количеству задач конкретного разработчика
  tuple<TasksInfo, TasksInfo> PerformPersonTasks(
      const string& person, int task_count);

private:
  map<string, TasksInfo> person_tasks_;
};

const TasksInfo& TeamTasks::GetPersonTasksInfo(const string& person) const {
  return person_tasks_.at(person);
}

void TeamTasks::AddNewTask(const string& person) {
  ++person_tasks_[person][TaskStatus::NEW];
}

// Функция для удаления нулей из словаря
void RemoveZeros(TasksInfo& tasks_info) {
  // Соберём те статусы, которые нужно убрать из словаря
  vector<TaskStatus> statuses_to_remove;
  for (const auto& task_item : tasks_info) {
    if (task_item.second == 0) {
      statuses_to_remove.push_back(task_item.first);
    }
  }
  for (const TaskStatus status : statuses_to_remove) {
    tasks_info.erase(status);
  }
}


tuple<TasksInfo, TasksInfo> TeamTasks::PerformPersonTasks(
    const string& person, int task_count) {
  TasksInfo updated_tasks, untouched_tasks;

  // Здесь и далее мы будем пользоваться тем фактом, что в std::map оператор []
  // в случае отсутствия ключа инициализирует значение по умолчанию,
  // если это возможно.
  // std::map::operator[] ->
  // http://ru.cppreference.com/w/cpp/container/map/operator_at
  TasksInfo& tasks = person_tasks_[person];

  // Посчитаем, сколько задач каждого из статусов нужно обновить,
  // пользуясь тем фактом, что по умолчанию enum class инциализирует значения
  // от нуля по возрастанию.
  // enum class -> http://ru.cppreference.com/w/cpp/language/enum
  for (TaskStatus status = TaskStatus::NEW;
       status != TaskStatus::DONE;
       status = Next(status)) {
    // Считаем обновлённые
    updated_tasks[Next(status)] = min(task_count, tasks[status]);
    // Считаем, сколько осталось обновить
    task_count -= updated_tasks[Next(status)];
  }

  // Обновляем статус текущих задач в соответствии с информацией об обновлённых
  // и находим количество нетронутых
  for (TaskStatus status = TaskStatus::NEW;
       status != TaskStatus::DONE;
       status = Next(status)) {
    untouched_tasks[status] = tasks[status] - updated_tasks[Next(status)];
    tasks[status] += updated_tasks[status] - updated_tasks[Next(status)];
  }
  // По условию, DONE задачи не нужно возвращать в не обновлённых задачах
  tasks[TaskStatus::DONE] += updated_tasks[TaskStatus::DONE];

  // По условию в словарях не должно быть нулей
  RemoveZeros(updated_tasks);
  RemoveZeros(untouched_tasks);

  return {updated_tasks, untouched_tasks};
}
 */

// Принимаем словарь по значению, чтобы иметь возможность
// обращаться к отсутствующим ключам с помощью [] и получать 0,
// не меняя при этом исходный словарь
void PrintTasksInfo(TasksInfo tasks_info) {
  cout << tasks_info[TaskStatus::NEW] << " new tasks" <<
      ", " << tasks_info[TaskStatus::IN_PROGRESS] << " tasks in progress" <<
      ", " << tasks_info[TaskStatus::TESTING] << " tasks are being tested" <<
      ", " << tasks_info[TaskStatus::DONE] << " tasks are done" << endl;
}

int main() {
	  TeamTasks tasks;
	  tasks.AddNewTask("Ilia");
	  for (int i = 0; i < 3; ++i) {
	    tasks.AddNewTask("Ivan");
	  }
	  cout << "Ilia's tasks: ";
	  PrintTasksInfo(tasks.GetPersonTasksInfo("Ilia"));
	  cout << "Ivan's tasks: ";
	  PrintTasksInfo(tasks.GetPersonTasksInfo("Ivan"));

	  TasksInfo updated_tasks, untouched_tasks;

	  tie(updated_tasks, untouched_tasks) =
	      tasks.PerformPersonTasks("Ivan", 2);
	  cout << "Updated Ivan's tasks: ";
	  PrintTasksInfo(updated_tasks);
	  cout << "Untouched Ivan's tasks: ";
	  PrintTasksInfo(untouched_tasks);

	  tie(updated_tasks, untouched_tasks) =
	      tasks.PerformPersonTasks("Ivan", 2);
	  cout << "Updated Ivan's tasks: ";
	  PrintTasksInfo(updated_tasks);
	  cout << "Untouched Ivan's tasks: ";
	  PrintTasksInfo(untouched_tasks);

/*
 * OUTPUT:
Ilia's tasks: 1 new tasks, 0 tasks in progress, 0 tasks are being tested, 0 tasks are done
Ivan's tasks: 3 new tasks, 0 tasks in progress, 0 tasks are being tested, 0 tasks are done
Updated Ivan's tasks: 0 new tasks, 2 tasks in progress, 0 tasks are being tested, 0 tasks are done
Untouched Ivan's tasks: 1 new tasks, 0 tasks in progress, 0 tasks are being tested, 0 tasks are done
Updated Ivan's tasks: 0 new tasks, 1 tasks in progress, 1 tasks are being tested, 0 tasks are done
Untouched Ivan's tasks: 0 new tasks, 1 tasks in progress, 0 tasks are being tested, 0 tasks are done
 */

	return 0;
}
//lalal